#ifndef EXTERNAL_API_H
#define EXTERNAL_API_H

#include <stopper.h>

// Dummy Request class
class Request
{
};

// Dummy GetRequest function
// NOLINTNEXTLINE (readability-identifier-naming)
Request *GetRequest(Stopper stopSignal) throw();

// Dummy ProcessRequest function
// NOLINTNEXTLINE (readability-identifier-naming)
void ProcessRequest(Request *request, Stopper stopSignal) throw();

#endif /* EXTERNAL_API_H */
