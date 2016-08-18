
"""
Shared Memory TeraSort Application to sort large files
(out-of memory size) based on key-value pair.

This makes use of Python Multiprocessing concept and uses child processes
to do the work.

The input raw data file is generated using GenSort library.


The basic design follows the usual MapReduce concept in Hadoop. Where, MAp phase 
consisting of Shuffle stage, then Combiner phase to sort raw data using HeapSort
and Reeducer phase where each sorted file is combined without taking it into memory.

Disk I/O are bottleneck to this programe as file size exceeds available memory size.

Author: Sunny Changediya

"""

import time
import sys
import os
from multiprocessing import Process
from collections import defaultdict
from itertools import izip
import re


memory_size = 2000000000
THREADS = 4

"""
	Sample code to check multi-Processing
"""

def sample(m, n):
	
    file_dict = {}
    with open('output.txt', 'a') as out_file:
		out_file.write('sample %d' %n)
		file_dict[n] = 'sample'+ str(n)
	
    print file_dict
       

# Reducer Function
# This is actual Merger function for Input chunks.
def reducer(m, n, fd_cnt):

	flag = 0
		
	try:
		fp1 = open('rec%d' %m, 'r')
		fp2 = open('rec%d' %n, 'r')
	except:
		# Only execeuted on even-odd file checking
		os.rename('rec%d' %m, 'rec%d' %fd_cnt)
		return
		
		
	outfile = open('rec%d' %fd_cnt, 'w+')
	
	line1 = fp1.readline()
	line2 = fp2.readline()
	
	while True:
		# read all files till end...
		if flag == 2:
			break
		
		# Check for any one file ends reading...
		# File1 or File2
		if not line1:
			flag += 1
			while len(line2):
				outfile.write(line2)
				line2 = fp2.readline()
			
		elif not line2:
			flag += 1
			while len(line1):
				outfile.write(line1)
				line1 = fp1.readline()

		# It means none of file has reached to its end...
		else: 
			
			if str(line1[:10]) < str(line2[:10]):
				outfile.write(line1)
				line1 = fp1.readline()
				
			elif str(line2[:10]) < str(line1[:10]):
				outfile.write(line2)
				line2 = fp2.readline()
				
			elif str(line2[:10]) == str(line1[:10]):
				outfile.write(line1)
				outfile.write(line2)
				line1 = fp1.readline()
				line2 = fp2.readline()
				
	# While loop ENds Here.....
	fp1.close()
	fp2.close()
	outfile.close()
	
	print("\nDeleting Files")
	os.remove('rec%d' %m)
	os.remove('rec%d' %n)
	
# Reducer Worker Function
# Handles the Process creation & Concurrency
def my_reducer(nfiles):
	
	fid = 0
	fd_cnt = nfiles - 1

	while nfiles > 1:
		
		procs = []
		for k in range(1, THREADS+1):
			
			# When no of chunks/Files is greater than Thread/process count
			# Used for early termination of child Processes
			if nfiles <= k:
				break	
			else:
				# call multiprocess...
				# Create Child processes...
				
				fd_cnt += 1
				child_proc = Process( target=reducer, args=(fid, fid+1, fd_cnt) )
				procs.append(child_proc)
				child_proc.start()
				fid = fid + 2
				nfiles = nfiles - 1
		
		
		# Join and wait for processes to finish...
		for n in procs:
			n.join()
			
	# While ends here...	

	

"""
	Sorting function, in abstraction viewed as a Combiner()
	for MapReduce in Hadoop.
	
	Shuffle all input chunks and sort them.
	
	Running time of HEAP Sort is O(n Log n)


"""

def max_heapify(array, root, heap_size):
	
	largest = root
	
	# left Child
	left = 2*root + 1
	
	# Right Child
	right = 2*root + 2 
	
	# Go till left child
	if left < heap_size and array[left] > array[largest]:
		largest = left
	
	# Go till right child
	if right < heap_size and array[right] > array[largest]:
		largest = right
	
	# Update the node & recursively process again
	if largest != root:
		
		temp = array[root]
		array[root] = array[largest]
		array[largest] = temp
		max_heapify(array, largest, heap_size)


# From right-most node, Build MAX HEAP
def build_max_heap(array, heap_size):
	
	# This gives right-most node on right tree
	rightmost = (heap_size - 2)//2
	
	for i in range( rightmost, -1, -1):
		max_heapify(array, i, heap_size)

# Actual HEAP Sort driver program
def heap_sort(array, heap_size):
	
	build_max_heap(array, heap_size)
	
	while heap_size > 1:
		
		temp = array[heap_size - 1]
		array[heap_size - 1] = array[0]
		array[0] = temp
		
		heap_size -= 1
		
		max_heapify(array, 0, heap_size)
		
	return array
	

# Worker function for MAP input chunks
# processes child process & parallel execution.
def worker(fid):
	
	file_dict = {}
	file_list = []
	
	with open('rec%d' %fid, 'r') as inp_file:
		
		for line in inp_file:
			key = str(line[:10])
			
			val = ''.join(line[12:])
			
			file_dict[key] = val
			file_list.append(key)
		
		
	# File data is in list. now go for sorting
	# Heap Sort starts here...
	
	sorted_list = heap_sort(file_list, len(file_list))
	
	# Get it into output file. So Disk I/O is done w/o taking into memory
	with open('rec%d' %fid, 'w+') as out_file:
		
		for key in sorted_list:	
			out_file.write(key+"  "+file_dict[key])			
		

# This is child process creation stage
def my_sorter(nfiles):
	
	i = 0
	
	
	while i < nfiles:
		
			
		procs = []
		for k in range(THREADS):
			
			# call multiprocess...
			# Create Child processes...
			if (i+k) >= nfiles:
				break
			else:
				
				child_proc = Process( target=worker, args=(i+k, ) )
				procs.append(child_proc)
				child_proc.start()
		
		
		# Join and wait for processes to finish...
		for n in procs:
			n.join()
			
		# increment count by Child process count
		i = i + THREADS
	
	# While ends here...	
		
		
# Raw file is broken down to chunks...
def make_file_chunks():	
	    
    fid = 0
    total_size = 0

    with open('datafile', 'r') as fp:
    	
        fptr = open('rec%d' %fid, 'w+')
    	
        for line in fp:
        	
            if total_size >= memory_size:
            	total_size = 0
                fptr.close()
                fid += 1
                fptr = open('rec%d' %fid, 'w+')

            fptr.write(line)
            total_size += 100
        
        fptr.close()


def main():
    
    # Input RAW "datafile" is generated using genSort library.
    
    file_size = os.path.getsize('datafile')
    
    nfiles = file_size//memory_size
    
    if nfiles == 0:
    	nfiles = 1
    
    print("file_size = %d & no of files made=%d" %(file_size, nfiles))
    
    
    # Divide the Input File to Chunks
    make_file_chunks()
    os.remove('datafile')
    print("\nGoing to Start Sorting")
    start_time = time.time()
    # The Input chunks goes to MAP function for Shuffle stage
    # Then, it goes to Combiner() function...
    my_sorter(nfiles)
    
    print("\nGoing to Reduce Sorting")
    # Reduce Phase: All sorted chunks are merged to isngle file...
    my_reducer(nfiles)
    
    end_time = time.time()
    print("\nTime Taken to Run %d Bytes is %s sec" %(file_size, end_time - start_time) )
    # Remove the RAW file & keep only sorted File...
    #os.remove('datafile')
    
    
if __name__ == '__main__':
    main()


