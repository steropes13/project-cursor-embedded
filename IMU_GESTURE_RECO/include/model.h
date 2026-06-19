#ifndef __MODEL_H__
#define __MODEL_H__

#ifdef __MODEL_NOT_PRESENT__
  #error "Model header file not found, using dummy model. Please run the training on the Jupyter notebook first"
  // const unsigned char model[12000] = {0x00}; 
#else
  #include "../../scratch/model.h"
#endif // __MODEL_NOT_PRESENT__

#endif // __MODEL_H__
