#
# LibNeubot interface - Public domain.
# WARNING: Autogenerated file - do not edit!
#

# pylint: disable = C0111, C0103

import _ctypes
import ctypes
import logging
import os
import sys

if sys.platform == "darwin":
    LIBNEUBOT_NAME = "/usr/local/lib/libneubot.dylib.3"
else:
    LIBNEUBOT_NAME = "/usr/local/lib/libneubot.so.3"

LIBNEUBOT = ctypes.CDLL(LIBNEUBOT_NAME)

DIE = getattr(os, '_exit')

NEUBOT_SLOT_VO = ctypes.CFUNCTYPE(None, ctypes.py_object)

NEUBOT_HOOK_VO = ctypes.CFUNCTYPE(None, ctypes.py_object)
NEUBOT_HOOK_VOS = ctypes.CFUNCTYPE(None, ctypes.py_object,
  ctypes.c_char_p)

#
# NeubotConnection API:
#

LIBNEUBOT.NeubotConnection_attach.restype = ctypes.c_void_p
LIBNEUBOT.NeubotConnection_attach.argtypes = (
    ctypes.c_void_p,
    ctypes.c_longlong,
)

def NeubotConnection_attach(proto, filenum):
    ret = LIBNEUBOT.NeubotConnection_attach(proto, filenum)
    if not ret:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_connect.restype = ctypes.c_void_p
LIBNEUBOT.NeubotConnection_connect.argtypes = (
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_char_p,
    ctypes.c_char_p,
)

def NeubotConnection_connect(proto, family, address, port):
    ret = LIBNEUBOT.NeubotConnection_connect(proto, family, address, 
      port)
    if not ret:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_get_protocol.restype = ctypes.c_void_p
LIBNEUBOT.NeubotConnection_get_protocol.argtypes = (
    ctypes.c_void_p,
)

def NeubotConnection_get_protocol(handle):
    ret = LIBNEUBOT.NeubotConnection_get_protocol(handle)
    if not ret:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_set_timeout.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_set_timeout.argtypes = (
    ctypes.c_void_p,
    ctypes.c_double,
)

def NeubotConnection_set_timeout(handle, timeo):
    ret = LIBNEUBOT.NeubotConnection_set_timeout(handle, timeo)
    if ret != 0:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_clear_timeout.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_clear_timeout.argtypes = (
    ctypes.c_void_p,
)

def NeubotConnection_clear_timeout(handle):
    ret = LIBNEUBOT.NeubotConnection_clear_timeout(handle)
    if ret != 0:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_start_tls.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_start_tls.argtypes = (
    ctypes.c_void_p,
    ctypes.c_uint,
)

def NeubotConnection_start_tls(handle, server_side):
    ret = LIBNEUBOT.NeubotConnection_start_tls(handle, server_side)
    if ret != 0:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_read.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_read.argtypes = (
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_size_t,
)

def NeubotConnection_read(handle, base, count):
    ret = LIBNEUBOT.NeubotConnection_read(handle, base, count)
    return ret

LIBNEUBOT.NeubotConnection_readline.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_readline.argtypes = (
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_size_t,
)

def NeubotConnection_readline(handle, base, count):
    ret = LIBNEUBOT.NeubotConnection_readline(handle, base, count)
    return ret

LIBNEUBOT.NeubotConnection_readn.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_readn.argtypes = (
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_size_t,
)

def NeubotConnection_readn(handle, base, count):
    ret = LIBNEUBOT.NeubotConnection_readn(handle, base, count)
    return ret

LIBNEUBOT.NeubotConnection_discardn.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_discardn.argtypes = (
    ctypes.c_void_p,
    ctypes.c_size_t,
)

def NeubotConnection_discardn(handle, count):
    ret = LIBNEUBOT.NeubotConnection_discardn(handle, count)
    return ret

LIBNEUBOT.NeubotConnection_write.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_write.argtypes = (
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_size_t,
)

def NeubotConnection_write(handle, base, count):
    ret = LIBNEUBOT.NeubotConnection_write(handle, base, count)
    if ret != 0:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_puts.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_puts.argtypes = (
    ctypes.c_void_p,
    ctypes.c_char_p,
)

def NeubotConnection_puts(handle, base):
    ret = LIBNEUBOT.NeubotConnection_puts(handle, base)
    if ret != 0:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_read_into_.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_read_into_.argtypes = (
    ctypes.c_void_p,
    ctypes.c_void_p,
)

def NeubotConnection_read_into_(handle, evdest):
    ret = LIBNEUBOT.NeubotConnection_read_into_(handle, evdest)
    if ret != 0:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_write_from_.restype = ctypes.c_int
LIBNEUBOT.NeubotConnection_write_from_.argtypes = (
    ctypes.c_void_p,
    ctypes.c_void_p,
)

def NeubotConnection_write_from_(handle, evsource):
    ret = LIBNEUBOT.NeubotConnection_write_from_(handle, evsource)
    if ret != 0:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotConnection_close.argtypes = (
    ctypes.c_void_p,
)

def NeubotConnection_close(handle):
    LIBNEUBOT.NeubotConnection_close(handle)

#
# NeubotEchoServer API:
#

LIBNEUBOT.NeubotEchoServer_construct.restype = ctypes.c_void_p
LIBNEUBOT.NeubotEchoServer_construct.argtypes = (
    PollerBase,
    ctypes.c_int,
    ctypes.c_char_p,
    ctypes.c_char_p,
)



LIBNEUBOT.NeubotPollable_construct.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPollable_construct.argtypes = (
    PollerBase,
    NEUBOT_SLOT_VO,
    NEUBOT_SLOT_VO,
    NEUBOT_SLOT_VO,
    ctypes.py_object,
)

def NeubotPollable_handle_read_slot_vo(selfptr):
    try:
        selfptr.handle_read()
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLABLE_HANDLE_READ_SLOT_VO = NEUBOT_SLOT_VO(
    NeubotPollable_handle_read_slot_vo
)

def NeubotPollable_handle_write_slot_vo(selfptr):
    try:
        selfptr.handle_write()
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLABLE_HANDLE_WRITE_SLOT_VO = NEUBOT_SLOT_VO(
    NeubotPollable_handle_write_slot_vo
)

def NeubotPollable_handle_error_slot_vo(selfptr):
    try:
        selfptr.handle_error()
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLABLE_HANDLE_ERROR_SLOT_VO = NEUBOT_SLOT_VO(
    NeubotPollable_handle_error_slot_vo
)



LIBNEUBOT.NeubotPollable_attach.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_attach.argtypes = (
    PollableBase,
    ctypes.c_longlong,
)



LIBNEUBOT.NeubotPollable_detach.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPollable_get_fileno.restype = ctypes.c_longlong
LIBNEUBOT.NeubotPollable_get_fileno.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPollable_set_readable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_set_readable.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPollable_unset_readable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_unset_readable.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPollable_set_writable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_set_writable.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPollable_unset_writable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_unset_writable.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPollable_set_timeout.argtypes = (
    PollableBase,
    ctypes.c_double,
)



LIBNEUBOT.NeubotPollable_clear_timeout.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPollable_close.argtypes = (
    PollableBase,
)



LIBNEUBOT.NeubotPoller_construct.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPoller_construct.argtypes = (
)



LIBNEUBOT.NeubotPoller_sched.restype = ctypes.c_int
LIBNEUBOT.NeubotPoller_sched.argtypes = (
    PollerBase,
    ctypes.c_double,
    NEUBOT_HOOK_VO,
    ctypes.py_object,
)

def NeubotPoller_sched_callback_hook_vo(closure):
    _ctypes.Py_DECREF(closure)
    try:
        closure.functions["callback"](closure.opaque)
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLER_SCHED_CALLBACK_HOOK_VO = NEUBOT_HOOK_VO(
    NeubotPoller_sched_callback_hook_vo
)



LIBNEUBOT.NeubotPoller_defer_read.restype = ctypes.c_int
LIBNEUBOT.NeubotPoller_defer_read.argtypes = (
    PollerBase,
    ctypes.c_longlong,
    NEUBOT_HOOK_VO,
    NEUBOT_HOOK_VO,
    ctypes.py_object,
    ctypes.c_double,
)

def NeubotPoller_defer_read_handle_ok_hook_vo(closure):
    _ctypes.Py_DECREF(closure)
    try:
        closure.functions["handle_ok"](closure.opaque)
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLER_DEFER_READ_HANDLE_OK_HOOK_VO = NEUBOT_HOOK_VO(
    NeubotPoller_defer_read_handle_ok_hook_vo
)

def NeubotPoller_defer_read_handle_timeout_hook_vo(closure):
    _ctypes.Py_DECREF(closure)
    try:
        closure.functions["handle_timeout"](closure.opaque)
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLER_DEFER_READ_HANDLE_TIMEOUT_HOOK_VO = NEUBOT_HOOK_VO(
    NeubotPoller_defer_read_handle_timeout_hook_vo
)



LIBNEUBOT.NeubotPoller_defer_write.restype = ctypes.c_int
LIBNEUBOT.NeubotPoller_defer_write.argtypes = (
    PollerBase,
    ctypes.c_longlong,
    NEUBOT_HOOK_VO,
    NEUBOT_HOOK_VO,
    ctypes.py_object,
    ctypes.c_double,
)

def NeubotPoller_defer_write_handle_ok_hook_vo(closure):
    _ctypes.Py_DECREF(closure)
    try:
        closure.functions["handle_ok"](closure.opaque)
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLER_DEFER_WRITE_HANDLE_OK_HOOK_VO = NEUBOT_HOOK_VO(
    NeubotPoller_defer_write_handle_ok_hook_vo
)

def NeubotPoller_defer_write_handle_timeout_hook_vo(closure):
    _ctypes.Py_DECREF(closure)
    try:
        closure.functions["handle_timeout"](closure.opaque)
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLER_DEFER_WRITE_HANDLE_TIMEOUT_HOOK_VO = NEUBOT_HOOK_VO(
    NeubotPoller_defer_write_handle_timeout_hook_vo
)



LIBNEUBOT.NeubotPoller_resolve.restype = ctypes.c_int
LIBNEUBOT.NeubotPoller_resolve.argtypes = (
    PollerBase,
    ctypes.c_char_p,
    ctypes.c_char_p,
    NEUBOT_HOOK_VOS,
    ctypes.py_object,
)

def NeubotPoller_resolve_callback_hook_vos(closure, string):
    _ctypes.Py_DECREF(closure)
    try:
        closure.functions["callback"](closure.opaque, string)
    except (KeyboardInterrupt, SystemExit):
        DIE(0)
    except:
        logging.error("Unhandled exception", exc_info=1)
        DIE(1)

NEUBOTPOLLER_RESOLVE_CALLBACK_HOOK_VOS = NEUBOT_HOOK_VOS(
    NeubotPoller_resolve_callback_hook_vos
)



LIBNEUBOT.NeubotPoller_loop.argtypes = (
    PollerBase,
)



LIBNEUBOT.NeubotPoller_break_loop.argtypes = (
    PollerBase,
)



#
# NeubotProtocol API:
#

LIBNEUBOT.NeubotProtocol_construct.restype = ctypes.c_void_p
LIBNEUBOT.NeubotProtocol_construct.argtypes = (
    ctypes.c_void_p,
    NEUBOT_SLOT_VO,
    NEUBOT_SLOT_VO,
    NEUBOT_SLOT_VO,
    NEUBOT_SLOT_VO,
    NEUBOT_SLOT_VO,
    NEUBOT_SLOT_VO,
    ctypes.py_object,
)

def NeubotProtocol_construct(poller, slot_connect, slot_ssl, slot_data, 
      slot_flush, slot_eof, slot_error, opaque):
    ret = LIBNEUBOT.NeubotProtocol_construct(poller, slot_connect, 
      slot_ssl, slot_data, slot_flush, slot_eof, slot_error, opaque)
    if not ret:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotProtocol_get_poller.restype = ctypes.c_void_p
LIBNEUBOT.NeubotProtocol_get_poller.argtypes = (
    ctypes.c_void_p,
)

def NeubotProtocol_get_poller(handle):
    ret = LIBNEUBOT.NeubotProtocol_get_poller(handle)
    if not ret:
        raise RuntimeError('LibNeubot error')
    return ret

LIBNEUBOT.NeubotProtocol_destruct.argtypes = (
    ctypes.c_void_p,
)

def NeubotProtocol_destruct(handle):
    LIBNEUBOT.NeubotProtocol_destruct(handle)

class NeubotHookClosure(object):
    def __init__(self):
        self.opaque = None
        self.functions = {}


#
# NeubotConnection wrapper:
#

class Connection(object):

    def __init__(self, proto, filenum):
        # We cannot destroy until the object is complete
        self._can_destroy = False
        self._context = LIBNEUBOT.NeubotConnection_attach(proto._context,
          filenum)
        if not self._context:
            raise RuntimeError('out of memory')
        # From now on we can destroy this object
        self._can_destroy = True
        LIBNEUBOT_OBJECTS.add(self)

    def __init__(self, proto, family, address, port):
        # We cannot destroy until the object is complete
        self._can_destroy = False
        self._context = LIBNEUBOT.NeubotConnection_connect(proto._context,
          family, address, port)
        if not self._context:
            raise RuntimeError('out of memory')
        # From now on we can destroy this object
        self._can_destroy = True
        LIBNEUBOT_OBJECTS.add(self)

    def get_protocol(self):
        return LIBNEUBOT.NeubotConnection_get_protocol(self._context)

    def set_timeout(self, timeo):
        retval = LIBNEUBOT.NeubotConnection_set_timeout(self._context, timeo)
        if retval != 0:
            raise RuntimeError('set_timeout failed')
        return retval

    def clear_timeout(self):
        retval = LIBNEUBOT.NeubotConnection_clear_timeout(self._context)
        if retval != 0:
            raise RuntimeError('clear_timeout failed')
        return retval

    def start_tls(self, server_side):
        retval = LIBNEUBOT.NeubotConnection_start_tls(self._context,
          server_side)
        if retval != 0:
            raise RuntimeError('start_tls failed')
        return retval

    def read(self, base, count):
        return LIBNEUBOT.NeubotConnection_read(self._context, base, count)

    def readline(self, base, count):
        return LIBNEUBOT.NeubotConnection_readline(self._context, base, count)

    def readn(self, base, count):
        return LIBNEUBOT.NeubotConnection_readn(self._context, base, count)

    def discardn(self, count):
        return LIBNEUBOT.NeubotConnection_discardn(self._context, count)

    def write(self, base, count):
        retval = LIBNEUBOT.NeubotConnection_write(self._context, base, count)
        if retval != 0:
            raise RuntimeError('write failed')
        return retval

    def puts(self, base):
        retval = LIBNEUBOT.NeubotConnection_puts(self._context, base)
        if retval != 0:
            raise RuntimeError('puts failed')
        return retval

    def read_into_(self, evdest):
        retval = LIBNEUBOT.NeubotConnection_read_into_(self._context,
          evdest._context)
        if retval != 0:
            raise RuntimeError('read_into_ failed')
        return retval

    def write_from_(self, evsource):
        retval = LIBNEUBOT.NeubotConnection_write_from_(self._context,
          evsource._context)
        if retval != 0:
            raise RuntimeError('write_from_ failed')
        return retval

    def close(self):
        if not self._can_destroy:
            return
        # Idempotent destructor for safety
        self._can_destroy = False
        LIBNEUBOT_OBJECTS.remove(self)
        LIBNEUBOT.NeubotConnection_close(self._context)

#
# NeubotEchoServer wrapper:
#

class EchoServer(EchoServerBase):

    def __init__(self, poller, use_ipv6, address, port):
        EchoServerBase.__init__(self)
        self.context_ = LIBNEUBOT.NeubotEchoServer_construct(poller, use_ipv6,
          address, port)
        if not self.context_:
            DIE(1)
        _ctypes.Py_INCREF(self)



class Pollable(PollableBase):

    def handle_read(self):
        pass

    def handle_write(self):
        pass

    def handle_error(self):
        pass

    def __init__(self, poller):
        PollableBase.__init__(self)
        self.context_ = LIBNEUBOT.NeubotPollable_construct(poller,
          NEUBOTPOLLABLE_HANDLE_READ_SLOT_VO,
          NEUBOTPOLLABLE_HANDLE_WRITE_SLOT_VO,
          NEUBOTPOLLABLE_HANDLE_ERROR_SLOT_VO, self)
        if not self.context_:
            DIE(1)
        _ctypes.Py_INCREF(self)

    def attach(self, filenum):
        return LIBNEUBOT.NeubotPollable_attach(self, filenum)

    def detach(self):
        LIBNEUBOT.NeubotPollable_detach(self)

    def get_fileno(self):
        return LIBNEUBOT.NeubotPollable_get_fileno(self)

    def set_readable(self):
        return LIBNEUBOT.NeubotPollable_set_readable(self)

    def unset_readable(self):
        return LIBNEUBOT.NeubotPollable_unset_readable(self)

    def set_writable(self):
        return LIBNEUBOT.NeubotPollable_set_writable(self)

    def unset_writable(self):
        return LIBNEUBOT.NeubotPollable_unset_writable(self)

    def set_timeout(self, delta):
        LIBNEUBOT.NeubotPollable_set_timeout(self, delta)

    def clear_timeout(self):
        LIBNEUBOT.NeubotPollable_clear_timeout(self)

    def close(self):
        if not self.context_:
            return
        _ctypes.Py_DECREF(self)
        LIBNEUBOT.NeubotPollable_close(self)
        self.context_ = None



class Poller(PollerBase):

    def __init__(self):
        PollerBase.__init__(self)
        self.context_ = LIBNEUBOT.NeubotPoller_construct()
        if not self.context_:
            DIE(1)
        _ctypes.Py_INCREF(self)

    def sched(self, delta, callback, opaque):

        closure = NeubotHookClosure()
        closure.functions['callback'] = callback
        closure.opaque = opaque
        _ctypes.Py_INCREF(closure)

        return LIBNEUBOT.NeubotPoller_sched(self, delta,
          NEUBOTPOLLER_SCHED_CALLBACK_HOOK_VO, closure)

    def defer_read(self, fileno, handle_ok, handle_timeout, opaque, timeout):

        closure = NeubotHookClosure()
        closure.functions['handle_ok'] = handle_ok
        closure.functions['handle_timeout'] = handle_timeout
        closure.opaque = opaque
        _ctypes.Py_INCREF(closure)

        return LIBNEUBOT.NeubotPoller_defer_read(self, fileno,
          NEUBOTPOLLER_DEFER_READ_HANDLE_OK_HOOK_VO,
          NEUBOTPOLLER_DEFER_READ_HANDLE_TIMEOUT_HOOK_VO, closure, timeout)

    def defer_write(self, fileno, handle_ok, handle_timeout, opaque,
          timeout):

        closure = NeubotHookClosure()
        closure.functions['handle_ok'] = handle_ok
        closure.functions['handle_timeout'] = handle_timeout
        closure.opaque = opaque
        _ctypes.Py_INCREF(closure)

        return LIBNEUBOT.NeubotPoller_defer_write(self, fileno,
          NEUBOTPOLLER_DEFER_WRITE_HANDLE_OK_HOOK_VO,
          NEUBOTPOLLER_DEFER_WRITE_HANDLE_TIMEOUT_HOOK_VO, closure, timeout)

    def resolve(self, family, name, callback, opaque):

        closure = NeubotHookClosure()
        closure.functions['callback'] = callback
        closure.opaque = opaque
        _ctypes.Py_INCREF(closure)

        return LIBNEUBOT.NeubotPoller_resolve(self, family, name,
          NEUBOTPOLLER_RESOLVE_CALLBACK_HOOK_VOS, closure)

    def loop(self):
        LIBNEUBOT.NeubotPoller_loop(self)

    def break_loop(self):
        LIBNEUBOT.NeubotPoller_break_loop(self)

#
# NeubotProtocol wrapper:
#

class Protocol(object):

    #
    # <Slots>
    #

    def slot_connect(self):
        pass

    @staticmethod
    def _slot_connect_(opaque):
        # pylint: disable = W0702
        try:
            opaque.slot_connect()
        except:
            logging.warning('Exception', exc_info=1)
            opaque.destruct()
        # pylint: enable = W0702

    def slot_ssl(self):
        pass

    @staticmethod
    def _slot_ssl_(opaque):
        # pylint: disable = W0702
        try:
            opaque.slot_ssl()
        except:
            logging.warning('Exception', exc_info=1)
            opaque.destruct()
        # pylint: enable = W0702

    def slot_data(self):
        pass

    @staticmethod
    def _slot_data_(opaque):
        # pylint: disable = W0702
        try:
            opaque.slot_data()
        except:
            logging.warning('Exception', exc_info=1)
            opaque.destruct()
        # pylint: enable = W0702

    def slot_flush(self):
        pass

    @staticmethod
    def _slot_flush_(opaque):
        # pylint: disable = W0702
        try:
            opaque.slot_flush()
        except:
            logging.warning('Exception', exc_info=1)
            opaque.destruct()
        # pylint: enable = W0702

    def slot_eof(self):
        pass

    @staticmethod
    def _slot_eof_(opaque):
        # pylint: disable = W0702
        try:
            opaque.slot_eof()
        except:
            logging.warning('Exception', exc_info=1)
            opaque.destruct()
        # pylint: enable = W0702

    def slot_error(self):
        pass

    @staticmethod
    def _slot_error_(opaque):
        # pylint: disable = W0702
        try:
            opaque.slot_error()
        except:
            logging.warning('Exception', exc_info=1)
            opaque.destruct()
        # pylint: enable = W0702

    #
    # </Slots>
    #

    def __init__(self, poller):
        self._c_slot_connect_ = NEUBOT_SLOT_VO(self._slot_connect_)
        self._c_slot_ssl_ = NEUBOT_SLOT_VO(self._slot_ssl_)
        self._c_slot_data_ = NEUBOT_SLOT_VO(self._slot_data_)
        self._c_slot_flush_ = NEUBOT_SLOT_VO(self._slot_flush_)
        self._c_slot_eof_ = NEUBOT_SLOT_VO(self._slot_eof_)
        self._c_slot_error_ = NEUBOT_SLOT_VO(self._slot_error_)
        self._c_self = ctypes.py_object(self)
        # We cannot destroy until the object is complete
        self._can_destroy = False
        self._context = LIBNEUBOT.NeubotProtocol_construct(poller._context,
          self._c_slot_connect_, self._c_slot_ssl_, self._c_slot_data_,
          self._c_slot_flush_, self._c_slot_eof_, self._c_slot_error_,
          self._c_self)
        if not self._context:
            raise RuntimeError('out of memory')
        # From now on we can destroy this object
        self._can_destroy = True
        LIBNEUBOT_OBJECTS.add(self)

    def get_poller(self):
        return LIBNEUBOT.NeubotProtocol_get_poller(self._context)

    def destruct(self):
        if not self._can_destroy:
            return
        # Idempotent destructor for safety
        self._can_destroy = False
        LIBNEUBOT_OBJECTS.remove(self)
        LIBNEUBOT.NeubotProtocol_destruct(self._context)

