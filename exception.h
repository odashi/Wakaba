// wakaba/exception.h
#ifndef WAKABA_EXCEPTION_H
#define WAKABA_EXCEPTION_H

namespace Wakaba {
	class Exception {};
	class InternalException : public Exception {};
	class ParameterException : public Exception {};
	class OutOfRangeException : public ParameterException {};
	class FormatException : public Exception {};
	class IOException : public Exception {};
	class NotImplementedException : public Exception {};

}

#endif
