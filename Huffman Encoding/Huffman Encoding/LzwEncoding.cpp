
#include "bstream.h"
#include "map.h"
void encodeFile(istream& infile, obstream& outfile) {
	string answ = "";
	int n = 255;
	Map<string, int> dict;
	string current = "";
	for (int i = 0; i <= n; i++) {
		dict.put(string(1, i), i);
	}
	char ch;
	while (infile.get(ch)) {
		string newStr = current + ch;
		if (!dict.containsKey(newStr)) {
			outfile << dict[current] << " ";
			dict[newStr] = ++n;
			current = ch;
		}
		else {
			current = newStr;
		}
	}
	if (!current.empty()) {
		outfile << dict[current] << " ";

	}
}
void decodeFile(ibstream& infile,ostream& file) {
	int n = 255;
	Map<int, string> dict;
	for (int i = 0; i <= n; i++) {
		dict.put(i, string(1, i));
	}
	int current = 0;
	int previous = -1;
	string answ;
	char ch;
	while (infile.get(ch)) {
		if (ch != ' ') {
			current = current * 10 + (ch - '0');
		}
		else {
			if (previous == -1) {
				file << dict[current];
				previous = current;
				current = 0;
			}
			else {
				dict[++n] = dict[previous] + dict[current][0];
				file << dict[current];
				previous = current;
				current = 0;
			}
		}
	}
	if (previous != -1 && dict.containsKey(current)) {
		file << dict[current];
	}
}
