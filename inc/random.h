#ifndef __RANDOM_H_
#define __RANDOM_H_

#define ur(n) (urand(0, n))
void init_random(void);
unsigned urand(unsigned min, unsigned max);
#endif
