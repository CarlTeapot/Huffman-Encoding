/**********************************************************
 * File: HuffmanEncoding.cpp
 *
 * Implementation of the functions from HuffmanEncoding.h.
 * Most (if not all) of the code that you write for this
 * assignment will go into this file.
 */

#include "HuffmanEncoding.h"

/* Function: getFrequencyTable
 * Usage: Map<ext_char, int> freq = getFrequencyTable(file);
 * --------------------------------------------------------
 * Given an input stream containing text, calculates the
 * frequencies of each character within that text and stores
 * the result as a Map from ext_chars to the number of times
 * that the character appears.
 *
 * This function will also set the frequency of the PSEUDO_EOF
 * character to be 1, which ensures that any future encoding
 * tree built from these frequencies will have an encoding for
 * the PSEUDO_EOF character.
 */
Map<ext_char, int> getFrequencyTable(istream& file) {
	Map<ext_char, int> map;
	char c;
	while (file.get(c)) {
		map[c]++;
	}
	map[PSEUDO_EOF]++;
	return map;	
}


Node* buildEncodingTree(Map<ext_char, int>& frequencies) {
		PriorityQueue<Node*> queue;
		for (ext_char c : frequencies) {
			Node* node = new Node;
			node->character = c;
			node->weight = frequencies[c];
			node->one = node->zero = NULL;
			queue.enqueue(node, frequencies[c]);
		}

		if (queue.size() == 1) {
			return NULL;
		}
		while (queue.size() > 1) {
			Node* n1 = queue.dequeue();
			Node* n2 = queue.dequeue();

			Node* par = new Node;
			par->character = NOT_A_CHAR;
			par->weight = n1->weight + n2->weight;
			par->one = n2;
			par->zero = n1;
			queue.enqueue(par, par->weight);
		}
		return queue.dequeue();
}



/* Function: freeTree
 * Usage: freeTree(encodingTree);
 * --------------------------------------------------------
 * Deallocates all memory allocated for a given encoding
 * tree.
 */
void freeTree(Node* root) {
	if (root == NULL)
		return;
	freeTree(root->one);
	freeTree(root->zero);
	delete(root);
}

/* Function: encodeFile
 * Usage: encodeFile(source, encodingTree, output);
 * --------------------------------------------------------
 * Encodes the given file using the encoding specified by the
 * given encoding tree, then writes the result one bit at a
 * time to the specified output file.
 *
 * This function can assume the following:
 *
 *   - The encoding tree was constructed from the given file,
 *     so every character appears somewhere in the encoding
 *     tree.
 *
 *   - The output file already has the encoding table written
 *     to it, and the file cursor is at the end of the file.
 *     This means that you should just start writing the bits
 *     without seeking the file anywhere.
 */ 
void encoderHelper(string current,Node* encodingTree, Map<ext_char,string>& map) {
	if (encodingTree == NULL) {
		return;
	}
	
	ext_char c = encodingTree->character;
	if (c != NOT_A_CHAR)
		map.put(c, current); 
	
	encoderHelper(current + "0", encodingTree->zero, map);
	encoderHelper(current + "1", encodingTree->one, map);

}
void encodeFile(istream& infile, Node* encodingTree, obstream& outfile) {
	Map<ext_char, string> encodingMap;
	encoderHelper("", encodingTree, encodingMap);
	char ch;
	while (infile.get(ch)) {
		for (char c : encodingMap[ch]) {
			int bit = 0;
			if (c == '1')
				bit = 1;
			outfile.writeBit(bit);
		}
	}
	for (char c : encodingMap[PSEUDO_EOF]) {
		int bit = 0;
		if (c == '1')
			bit = 1;
		outfile.writeBit(bit);
	}
}
/* Function: decodeFile
 * Usage: decodeFile(encodedFile, encodingTree, resultFile);
 * --------------------------------------------------------
 * Decodes a file that has previously been encoded using the
 * encodeFile function.  You can assume the following:
 *
 *   - The encoding table has already been read from the input
 *     file, and the encoding tree parameter was constructed from
 *     this encoding table.
 *
 *   - The output file is open and ready for writing.
 */

void decodeFile(ibstream& infile, Node* encodingTree, ostream& file) {
	Node* iterator = encodingTree;
	int bt = infile.readBit();
	while (bt != -1) {
		if (bt == 0) {
			iterator = iterator->zero;
		}
		else {
			iterator = iterator->one;
		}
		bt = infile.readBit();
		if (iterator->one == NULL && iterator->zero == NULL) {
			if (iterator->character != PSEUDO_EOF) {
				file.put(iterator->character);
				iterator = encodingTree;
			}
			else {
				break;
			}
		}
	}
}


/* Function: writeFileHeader
 * Usage: writeFileHeader(output, frequencies);
 * --------------------------------------------------------
 * Writes a table to the front of the specified output file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to decompress input files once they've been
 * compressed.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * readFileHeader function defined below this one so that it
 * can properly read the data back.
 */
void writeFileHeader(obstream& outfile, Map<ext_char, int>& frequencies) {
	/* The format we will use is the following:
	 *
	 * First number: Total number of characters whose frequency is being
	 *               encoded.
	 * An appropriate number of pairs of the form [char][frequency][space],
	 * encoding the number of occurrences.
	 *
	 * No information about PSEUDO_EOF is written, since the frequency is
	 * always 1.
	 */
	 
	/* Verify that we have PSEUDO_EOF somewhere in this mapping. */
	if (!frequencies.containsKey(PSEUDO_EOF)) {
		error("No PSEUDO_EOF defined.");
	}
	
	/* Write how many encodings we're going to have.  Note the space after
	 * this number to ensure that we can read it back correctly.
	 */
	outfile << frequencies.size() - 1 << ' ';
	
	/* Now, write the letter/frequency pairs. */
	foreach (ext_char ch in frequencies) {
		/* Skip PSEUDO_EOF if we see it. */
		if (ch == PSEUDO_EOF) continue;
		
		/* Write out the letter and its frequency. */
		outfile << char(ch) << frequencies[ch] << ' ';
	}
}

/* Function: readFileHeader
 * Usage: Map<ext_char, int> freq = writeFileHeader(input);
 * --------------------------------------------------------
 * Reads a table to the front of the specified input file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to reconstruct the encoding tree for that file.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * writeFileHeader function defined before this one so that it
 * can properly write the data.
 */
Map<ext_char, int> readFileHeader(ibstream& infile) {
	/* This function inverts the mapping we wrote out in the
	 * writeFileHeader function before.  If you make any
	 * changes to that function, be sure to change this one
	 * too!
	 */
	Map<ext_char, int> result;
	
	/* Read how many values we're going to read in. */
	int numValues;
	infile >> numValues;
	
	/* Skip trailing whitespace. */
	infile.get();
	
	/* Read those values in. */
	for (int i = 0; i < numValues; i++) {
		/* Get the character we're going to read. */
		ext_char ch = infile.get();
		
		/* Get the frequency. */
		int frequency;
		infile >> frequency;
		
		/* Skip the space character. */
		infile.get();
		
		/* Add this to the encoding table. */
		result[ch] = frequency;
	}
	
	/* Add in 1 for PSEUDO_EOF. */
	result[PSEUDO_EOF] = 1;
	return result;
}

void compress(ibstream& infile, obstream& outfile) {
	Map<ext_char, int> frequencies = getFrequencyTable(infile);
	writeFileHeader(outfile, frequencies);
	infile.rewind();
	Node* root = buildEncodingTree(frequencies);
	encodeFile(infile, root, outfile);
	freeTree(root);
}

void decompress(ibstream& infile, ostream& outfile) {
	Map<ext_char, int> frequencies = readFileHeader(infile);
	Node* root = buildEncodingTree(frequencies);
	decodeFile(infile, root, outfile);
	freeTree(root);
}

