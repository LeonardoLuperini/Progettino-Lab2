#include <stdlib.h>
#include <math.h>

/* avg stand for AVeraGe
 *
 * Params:
 * array of doubles,
 * len of the array.
 *
 * Returns:
 * The average
 */
double avg(double numbers[], size_t len);

/* std stand for STandard Deviation
 *
 * Params:
 * array of doubles,
 * len of the array.
 *
 * Returns:
 * The stadard Deviation
 *
 * NOTE:
 * this function close the program 
 * if gets an error
 */
double std(double numbers[], size_t len);
