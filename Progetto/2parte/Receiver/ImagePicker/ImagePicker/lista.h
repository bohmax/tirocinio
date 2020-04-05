//
//  lista.h
//  ImagePicker
//
//  Created by Massimo Puddu on 04/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef lista_h
#define lista_h

#include "utility.h"

void pushList(list** head, list** coda, AVFrame* frame, int nFrame);

list* popList(list** head, list** coda);

void freeList(list** el);

#endif /* lista_h */
