# Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

DESTDIR ?= /usr/local
MANDIR = $(DESTDIR)/share/man

all debug clean:
	$(MAKE) -C src/ax $@
	$(MAKE) -C src/ut $@

install:
	install -m 755 -d \
		$(DESTDIR)/lib \
		$(DESTDIR)/include \
		$(DESTDIR)/include/ax \
		$(DESTDIR)/include/ax/type \
		$(DESTDIR)/include/ax/static \
		$(DESTDIR)/include/ut \
		$(MANDIR)/man3
	install -m 644 lib/libax.a lib/libut.a $(DESTDIR)/lib
	install -m 644 include/ax/*.h $(DESTDIR)/include/ax
	install -m 644 include/ax/type/*.h $(DESTDIR)/include/ax/type
	install -m 644 include/ax/static/*.h $(DESTDIR)/include/ax/type/static
	install -m 644 include/ut/*.h $(DESTDIR)/include/ut
	install -m 644 man/man3/* $(MANDIR)/man3

uninstall:
	$(RM) -r $(DESTDIR)/include/ax $(DESTDIR)/include/ut
	$(RM) $(DESTDIR)/include/ax.h $(DESTDIR)/include/ut.h $(DESTDIR)/lib/libax.a $(DESTDIR)/lib/libut.a
	$(RM) $(MANDIR)/share/man/man3/ax_*.3

.PHONY: all debug clean install uninstall
