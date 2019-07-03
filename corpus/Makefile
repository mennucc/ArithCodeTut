all: count_webster.c count_dickens.c count_book1.c

.INTERMEDIATE: silesia/webster silesia/dickens calgary_corpus/book1

calgary_corpus/book1:
	test -d calgary_corpus  || mkdir calgary_corpus
	cd calgary_corpus && wget http://www.data-compression.info/files/corpora/calgarycorpus.zip
	cd calgary_corpus && unzip calgarycorpus.zip
	echo you may want to delete calgarycorpus.zip


silesia/webster:
silesia/dickens:
	# see http://sun.aei.polsl.pl/~sdeor/index.php?page=silesia
	test -d silesia  || mkdir silesia
	cd silesia && wget http://sun.aei.polsl.pl/~sdeor/corpus/silesia.zip
	cd silesia && unzip silesia.zip
	echo you may want to delete silesia.zip


count_book1.c:  calgary_corpus/book1
	 ./letter_frequencies.py calgary_corpus/book1 > count_book1.c

count_webster.c: silesia/dickens
	 ./letter_frequencies.py silesia/webster > count_webster.c

count_dickens.c: silesia/dickens
	 ./letter_frequencies.py silesia/dickens > count_dickens.c
