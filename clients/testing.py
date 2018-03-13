import memcached_udp

import time

current_milli_time = lambda: int(round(time.time() * 1000000))
current_milli_time_1 = lambda: time.time()

client = memcached_udp.Client([('192.168.0.102', 11211)])


a = ''
for i in range(3000):
	a = a + str(i)	
#client.set('key1', a)





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

#while(1):
#	print(current_milli_time_1())
#	a = gettimeofday()
#	r = client.get('key1')
#	b = gettimeofday()
#	print((b - a)*1000000)

r = client.get("kay")
print(r)
