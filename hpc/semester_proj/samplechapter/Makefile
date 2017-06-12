all:
	latex sample.tex
	make substitute

index:
	bibtex sample
	makeindex index
	makeindex const
	makeindex op
	makeindex author
	makeindex rout

substitute:
	sed s/{section}{Index}/{chapter}{Index}/ < sample.toc > /tmp/sample.toc
	mv /tmp/sample.toc sample.toc
	sed s/{section}{Operation/{chapter}{Operation/ < sample.toc > /tmp/sample.toc
	mv /tmp/sample.toc sample.toc
	sed s/{section}{Function/{chapter}{Function/ < sample.toc > /tmp/sample.toc
	mv /tmp/sample.toc sample.toc
	sed s/{section}{Constant/{chapter}{Constant/ < sample.toc > /tmp/sample.toc
	mv /tmp/sample.toc sample.toc
	sed s/{section}{Author/{chapter}{Author/ < sample.toc > /tmp/sample.toc
	mv /tmp/sample.toc sample.toc

clean:
	rm *~ *.aux *.idx *.log *.toc */*~
