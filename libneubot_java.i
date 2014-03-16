
%module(directors="1") LibNeubotJava;
%feature("director") Neubot::Stream;

//
// Pass a byte array from C to hava. To keep the array safe,
// we copy it into a newly allocated ByteArray.
//
%typemap(out) Neubot::BytesContainer peek {
    if (result.base != NULL && result.len > 0) {
        jresult = JCALL1(NewByteArray, jenv, result.len);
        if (jresult) JCALL4(SetByteArrayRegion, jenv, jresult, 0, result.len, result.base);
    }
}
%typemap(jtype) Neubot::BytesContainer peek "ByteArray"
%typemap(jstype) Neubot::BytesContainer peek "ByteArray"
%typemap(jni) Neubot::BytesContainer peek "jbyteArray"
%typemap(javain) Neubot::BytesContainer peek "$javainput"

//
// Pass a byte array from Java to C. It is the called function's
// responsibility to copy the array to keep it safe.
//
%apply (char *STRING, size_t LENGTH) { (const char *bytes, size_t count) }; 

%include "bucket.hh"
%include "connector.hh"
%include "stream.hh"

%{
#include "../bucket.hh"
#include "../connector.hh"
#include "../stream.hh"
%}
