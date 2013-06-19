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
 * Minimalist & Fast UTF8 Decoder Header
 *
 */
 
#ifndef MINUTF8_H
#define MINUTF8_H

int             utf8_len(const char *s);
void            utf8_dec_ex(int * d, int dl, const char * s);
int *           utf8_dec(const char *s);
int             utf8c(const char * src, const char ** ss,int * move);

#endif