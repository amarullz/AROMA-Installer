/*
 * Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Descriptions:
 * -------------
 * AROMA Assosiative Array
 *
 */

#include "../aroma.h"

AARRAYP aarray_create() {
  AARRAYP a = (AARRAYP) malloc(sizeof(AARRAY));
  a->length = 0;
  a->items  = malloc(1);
  return a;
}

char * aarray_get(AARRAYP a, char * key) {
  int i;
  
  if (!a || !key) {
    return NULL;
  }
  
  for (i = 0; i < a->length; i++) {
    if (strcmp(a->items[i].key, key) == 0) {
      return a->items[i].val;
    }
  }
  
  return NULL;
}

byte aarray_set(AARRAYP a, char * key, char * val) {
  int found_id = -1;
  int i;
  
  if (!a || !val || !key) {
    return 0;
  }
  
  for (i = 0; i < a->length; i++) {
    if (strcmp(a->items[i].key, key) == 0) {
      found_id = i;
      break;
    }
  }
  
  if (found_id != -1) {
    if (a->items[found_id].val != NULL) {
      free(a->items[found_id].val);
    }
    
    a->items[found_id].val = malloc(strlen(val) + 1);
    strcpy(a->items[found_id].val, val);
    return 1;
  }
  else {
    //-- Find Freed Items
    for (i = 0; i < a->length; i++) {
      if (a->items[i].key == NULL) {
        found_id = i;
        break;
      }
    }
    
    if (found_id == -1) {
      found_id = a->length;
      a->length++;
      a->items = realloc(a->items, sizeof(AARRAY) * a->length);
    }
    
    a->items[found_id].key = malloc(strlen(key) + 1);
    a->items[found_id].val = malloc(strlen(val) + 1);
    strcpy(a->items[found_id].key, key);
    strcpy(a->items[found_id].val, val);
    return 1;
  }
}

byte aarray_del(AARRAYP a, char * key) {
  int found_id = -1;
  int i;
  
  if (!a || !key) {
    return 0;
  }
  
  for (i = 0; i < a->length; i++) {
    if (strcmp(a->items[i].key, key) == 0) {
      free(a->items[i].key);
      free(a->items[i].val);
      a->items[i].key = NULL;
      a->items[i].val = NULL;
      return 1;
    }
  }
  
  return 0;
}

byte aarray_free(AARRAYP a) {
  int i;
  
  if (!a) {
    return 0;
  }
  
  for (i = 0; i < a->length; i++) {
    if (a->items[i].key != NULL) {
      free(a->items[i].key);
      free(a->items[i].val);
    }
    
    a->items[i].key = NULL;
    a->items[i].val = NULL;
  }
  
  free(a->items);
  free(a);
  return 1;
}