ALL THE FUNCTION CALLS SHOULD BE DONE AS INDICATED IN MAIN FUNCTION.

PrintFileList() AND PrintFAT() FUNCTIONS CREATES SEPERATE .TXT FILES YOU CAN CHECK THE VALUES FROM THESE FILES.

EXCEPT FROM SEARCH() AND LIST() FUNCTION, YOU SHOULD CALL PrintFileList() AND PrintFAT() TO OBSERVE DIFFERENCES

USAGE OF FUNCTIONS

Firstly create disk.image: dd if=/dev/zero of=disk.image bs=2146304 count=1
Next compile the source code : gcc myfs.c -o myfs -lm

Then:

For Format:	./myfs disk.image -format

For Write:	./myfs disk.image -write source_file dest_file

For Read:	./myfs disk.image -read source_file dest_file

For Delete:	./myfs disk.image -delete source_file
	
For List:	./myfs disk.image -list

For Sort:	./myfs disk.image -sorta

For Rename:	./myfs disk.image -rename source_file new_name

For Duplicate:	./myfs disk.image -duplicate source_file

For Search:	./myfs disk.image -search source_file

For Hide:	./myfs disk.image -hide source_file

For Unhide:	./myfs disk.image -unhide source_file

For PrintFileList:	./myfs disk.image -printfilelist

For PrintFAT:	./myfs disk.image -printfat

For Defragment:	./myfs disk.image -defragment
