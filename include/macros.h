/*
* Defines commonly used macros
* Ref : //macro to check a feature , ref : https://stackoverflow.com/questions/18348625/c-macro-to-enable-and-disable-code-features
*/


#ifndef MACROS_H
#define MACROS_H

#ifndef IN_USE 
  #define IN_USE == 1
#endif
#ifndef NOT_IN_USE 
  #define NOT_IN_USE == 0
#endif
#ifndef USING
  #define USING(feature) 1 feature
#endif

#endif