//
//  define.h
//  ImagePicker
//
//  Created by Massimo Puddu on 31/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef define_h
#define define_h

#define ERRORSYSHEANDLER(r,c,d,e) if((r=c)==d) { perror(e);exit(errno); }
#define SYSFREE(r,c,d,e) if((r=c)!=d) { perror(e);exit(errno); }
#define DIM 500 //dimensione media per 2 di sps e pps
#define GOPM 400000 //dimensioni di un GOP medio moltiplicato per 2
#define NUMDECODERTHR 2 //numero di thread per decodificare le immagini

#endif /* define_h */
