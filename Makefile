all:
	gcc flv.c -o flv
	gcc flv_segmenter.c -o flv_segmenter
clean:
	rm -rf flv
	rm -rf flv_segmenter
