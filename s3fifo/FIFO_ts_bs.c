//
//  first in first out
//
//
//  FIFO.c
//  libCacheSim
//
//  Created by Juncheng on 12/4/18.
//  Copyright © 2018 Juncheng. All rights reserved.
//

// in this version, we use time stamp to find the position

#include "../../dataStructure/hashtable/hashtable.h"
#include "../../include/libCacheSim/evictionAlgo.h"

#ifdef __cplusplus
extern "C" {
#endif

// ***********************************************************************
// ****                                                               ****
// ****                   function declarations                       ****
// ****                                                               ****
// ***********************************************************************

static void FIFO_parse_params(cache_t *cache,
                              const char *cache_specific_params);
static void FIFO_free(cache_t *cache);
static bool FIFO_get(cache_t *cache, const request_t *req);
static cache_obj_t *FIFO_find(cache_t *cache, const request_t *req,
                              const bool update_cache);
static cache_obj_t *FIFO_insert(cache_t *cache, const request_t *req);
static cache_obj_t *FIFO_to_evict(cache_t *cache, const request_t *req);
static void FIFO_evict(cache_t *cache, const request_t *req);
static bool FIFO_remove(cache_t *cache, const obj_id_t obj_id);

// ***********************************************************************
// ****                                                               ****
// ****                   end user facing functions                   ****
// ****                                                               ****
// ****                       init, free, get                         ****
// ***********************************************************************

/**
 * @brief initialize a ARC cache
 *
 * @param ccache_params some common cache parameters
 * @param cache_specific_params ARC specific parameters, should be NULL
 */
cache_t *FIFO_init(const common_cache_params_t ccache_params,
                   const char *cache_specific_params) {
  cache_t *cache = cache_struct_init("FIFO", ccache_params, cache_specific_params);
  cache->cache_init = FIFO_init;
  cache->cache_free = FIFO_free;
  cache->get = FIFO_get;
  cache->find = FIFO_find;
  cache->insert = FIFO_insert;
  cache->evict = FIFO_evict;
  cache->remove = FIFO_remove;
  cache->to_evict = FIFO_to_evict;
  cache->get_occupied_byte = cache_get_occupied_byte_default;
  cache->get_n_obj = cache_get_n_obj_default;
  cache->can_insert = cache_can_insert_default;
  cache->obj_md_size = 0;

  cache->eviction_params = malloc(sizeof(FIFO_params_t));
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
  params->q_head = NULL;
  params->q_tail = NULL;
  
  // get the array for position counting
  params->hit_pos = (uint64_t*)cache_specific_params;
  params->has_obj_removed = false;
  params->logic_timer = 0;

  params->removed_track.capacity = 16;
  params->removed_track.n_removed = 0;
  params->removed_track.removed_timestamps = 
      (uint64_t*)malloc(sizeof(uint64_t) * params->removed_track.capacity);
  
  return cache;
}

/**
 * free resources used by this cache
 *
 * @param cache
 */
static void FIFO_free(cache_t *cache) {
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
  free(params->removed_track.removed_timestamps);
  free(cache->eviction_params);
  cache_struct_free(cache);
}

/**
 * @brief this function is the user facing API
 * it performs the following logic
 *
 * ```
 * if obj in cache:
 *    update_metadata
 *    return true
 * else:
 *    if cache does not have enough space:
 *        evict until it has space to insert
 *    insert the object
 *    return false
 * ```
 *
 * @param cache
 * @param req
 * @return true if cache hit, false if cache miss
 */
static bool FIFO_get(cache_t *cache, const request_t *req) {
  return cache_get_base(cache, req);
}

// ***********************************************************************
// ****                                                               ****
// ****       developer facing APIs (used by cache developer)         ****
// ****                                                               ****
// ***********************************************************************

/**
 * @brief find an object in the cache
 *
 * @param cache
 * @param req
 * @param update_cache whether to update the cache,
 *  if true, the object is promoted
 *  and if the object is expired, it is removed from the cache
 * @return the object or NULL if not found
 */
static cache_obj_t *FIFO_find(cache_t *cache, const request_t *req,
                               const bool update_cache) {

  if (update_cache) { 
    cache_obj_t *obj = cache_find_base(cache, req, false);
    // if miss
    if (obj == NULL) return cache_find_base(cache, req, update_cache);
    // if hit, get position
    FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
    // here only main and small
    uint64_t position = params->logic_timer - obj->logic_timestamp - 1;
    params->hit_pos[position]++;
  }
  return cache_find_base(cache, req, update_cache);
}

/**
 * @brief insert an object into the cache,
 * update the hash table and cache metadata
 * this function assumes the cache has enough space
 * and eviction is not part of this function
 *
 * @param cache
 * @param req
 * @return the inserted object
 */
static cache_obj_t *FIFO_insert(cache_t *cache, const request_t *req) {
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
  cache_obj_t *obj = cache_insert_base(cache, req);
  obj->logic_timestamp = params->logic_timer;
  prepend_obj_to_head(&params->q_head, &params->q_tail, obj);
  params->logic_timer++;
  return obj;
}

/**
 * @brief find the object to be evicted
 * this function does not actually evict the object or update metadata
 * not all eviction algorithms support this function
 * because the eviction logic cannot be decoupled from finding eviction
 * candidate, so use assert(false) if you cannot support this function
 *
 * @param cache the cache
 * @return the object to be evicted
 */
static cache_obj_t *FIFO_to_evict(cache_t *cache, const request_t *req) {
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
  return params->q_tail;
}

/**
 * @brief evict an object from the cache
 * it needs to call cache_evict_base before returning
 * which updates some metadata such as n_obj, occupied size, and hash table
 *
 * @param cache
 * @param req not used
 * @param evicted_obj if not NULL, return the evicted object to caller
 */
static void FIFO_evict(cache_t *cache, const request_t *req) {
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
  cache_obj_t *obj_to_evict = params->q_tail;
  DEBUG_ASSERT(params->q_tail != NULL);

  // we can simply call remove_obj_from_list here, but for the best performance,
  // we chose to do it manually
  // remove_obj_from_list(&params->q_head, &params->q_tail, obj);

  params->q_tail = params->q_tail->queue.prev;
  if (likely(params->q_tail != NULL)) {
    params->q_tail->queue.next = NULL;
  } else {
    /* cache->n_obj has not been updated */
    DEBUG_ASSERT(cache->n_obj == 1);
    params->q_head = NULL;
  }

  cache_evict_base(cache, obj_to_evict, true);
}

/**
 * @brief remove an object from the cache
 * this is different from cache_evict because it is used to for user trigger
 * remove, and eviction is used by the cache to make space for new objects
 *
 * it needs to call cache_remove_obj_base before returning
 * which updates some metadata such as n_obj, occupied size, and hash table
 *
 * @param cache
 * @param obj_id
 * @return true if the object is removed, false if the object is not in the
 * cache
 */
static bool FIFO_remove(cache_t *cache, const obj_id_t obj_id) {
  FIFO_params_t *params = (FIFO_params_t *)cache->eviction_params;
  
  cache_obj_t *obj = hashtable_find_obj_id(cache->hashtable, obj_id);
  if (obj == NULL) {
    return false;
  }

  // ghost hit happened here
  // need to consider the case some objs are removed
  uint64_t raw_position = params->logic_timer - obj->logic_timestamp - 1;
      
  // compute the number of removed objects in the range
  uint64_t n_removed_in_range = 0;

  if (params->has_obj_removed) {
    // 找到第一个大于obj->logic_timestamp的元素
    uint64_t left = 0, right = params->removed_track.n_removed;
    while (left < right) {
      uint64_t mid = left + (right - left) / 2;
      if (params->removed_track.removed_timestamps[mid] <= obj->logic_timestamp)
        left = mid + 1;
      else
        right = mid;
    }
    uint64_t start_idx = left;
    
    // 找到第一个大于等于params->logic_timer的元素
    left = 0, right = params->removed_track.n_removed;
    while (left < right) {
      uint64_t mid = left + (right - left) / 2;
      if (params->removed_track.removed_timestamps[mid] < params->logic_timer)
        left = mid + 1;
      else
        right = mid;
    }
    uint64_t end_idx = left;
    
    // 计算区间内元素数量
    n_removed_in_range = (end_idx > start_idx) ? (end_idx - start_idx) : 0;
  }
      
  // adjust the position
  uint64_t adjusted_position = raw_position - n_removed_in_range;
      
  // update the position
  if ((int64_t)adjusted_position < cache->cache_size) {
    params->hit_pos[adjusted_position]++;
  } 


  // record the position of the removed object
  if (params->removed_track.n_removed >= params->removed_track.capacity) {
    // extend the array
    params->removed_track.capacity *= 2;
    params->removed_track.removed_timestamps = 
        (uint64_t*)realloc(params->removed_track.removed_timestamps, 
                         sizeof(uint64_t) * params->removed_track.capacity);
  }
  
  // add the timestamp to the removed track
  uint64_t timestamp = obj->logic_timestamp;
  params->removed_track.removed_timestamps[params->removed_track.n_removed++] = timestamp;
  
  // sort
  int i = params->removed_track.n_removed - 1;
  while (i > 0 && params->removed_track.removed_timestamps[i-1] > timestamp) {
    params->removed_track.removed_timestamps[i] = params->removed_track.removed_timestamps[i-1];
    i--;
  }
  params->removed_track.removed_timestamps[i] = timestamp;
  
  params->has_obj_removed = true;
  // printf("remove obj %lu\n", obj->obj_id);

  remove_obj_from_list(&params->q_head, &params->q_tail, obj);
  cache_remove_obj_base(cache, obj, true);

  return true;
}

static void FIFO_parse_params(cache_t *cache,
                              const char *cache_specific_params) {
  if (cache_specific_params != NULL) {
    ERROR("FIFO does not support any cache specific parameters\n");
    exit(1);
  }
}

#ifdef __cplusplus
}
#endif
