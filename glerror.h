//
//	based on check_gl_error by Morten Nobel-Jørgensen, 
//  http://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
//

#ifndef GLERROR_H
#define GLERROR_H

void _check_gl_error(const char *file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///

#ifdef GLERROR_ENABLE
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
#else
#define check_gl_error()
#endif

#endif // GLERROR_H