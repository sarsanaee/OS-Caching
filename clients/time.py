_gettimeofday = None
def gettimeofday():
	import ctypes
	global _gettimeofday
	
	if not _gettimeofday:
		_gettimeofday = ctypes.cdll.LoadLibrary("libc.so.6").gettimeofday
	
	class timeval(ctypes.Structure):
		_fields_ = \
		[
			("tv_sec", ctypes.c_long),
			("tv_usec", ctypes.c_long)
		]
	
	tv = timeval()
	
	_gettimeofday(ctypes.byref(tv), None)
	
	return float(tv.tv_sec) + (float(tv.tv_usec) / 1000000)

print(gettimeofday())
