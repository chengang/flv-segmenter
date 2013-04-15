all:
	gcc flv.c -o flv
	gcc flv_segmenter.c -o flv_segmenter
clean:
	rm -rf flv
	rm -rf flv_segmenter
install:
	install -c flv /usr/bin
	install -c flv_segmenter /usr/bin
uninstall:
	rm -f /usr/bin/flv
	rm -f /usr/bin/flv_segmenter
