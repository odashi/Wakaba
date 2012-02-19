// wakaba/dllexport.h
#ifndef WAKABA_DLLEXPORT_H
#define WAKABA_DLLEXPORT_H

#ifdef WAKABA_DLL
#define WAKABA_API __declspec(dllexport)
#else
#define WAKABA_API __declspec(dllimport)
#endif

#endif
