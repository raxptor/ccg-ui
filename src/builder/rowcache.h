#include <vector>
#include <map>

struct row
{
	unsigned int totlen;
	unsigned char start;
	std::vector<char> data;
	unsigned int uses;
};

struct row_length
{
	std::vector<row> rows;
};

typedef std::map<unsigned int, row_length> row_cache;

int row_cache_compress(row *in, row *out, char *out_bytes)
{
	int bytes = 0;
	
	for (int i=0;i<in->data.size();i++)
	{
		if (in->data[i] != out->data[i])
		{
			bytes += 2;
		}
	}
	return bytes;
}

void row_cache_optimize(row_cache *cache)
{
	row_cache::iterator i = cache->begin();
	while (i != cache->end())
	{
		row_length newout;
		row_length & input = i->second;
		// take first
		std::cout << "comprezion with " << input.rows.size() << std::endl;
		newout.rows.push_back(input.rows[0]);
		input.rows.erase(input.rows.begin());
		
		while (!input.rows.empty())
		{
			// choose smallest compressed from last output to next input
			int smallest, diff;
			for (unsigned int k=0;k<input.rows.size();k++)
			{
				char bytes[1024];
				int d = row_cache_compress(&newout.rows[newout.rows.size()-1], &input.rows[k], bytes);
				if (!k || d < diff)
				{
					diff = d;
					smallest = k;
				}
			}
			
			std::cout << "Compressed from " << newout.rows.size() << " to " << smallest << " with " << diff << " bytes (" << i->first << ")" << std::endl;
			newout.rows.push_back(input.rows[smallest]);
			input.rows.erase(input.rows.begin() + smallest);
		}
		
		i->second.rows = newout.rows;
		i++;
	}
}

void row_cache_insert(row_cache *cache, row *rd)
{
	row_cache::iterator i = cache->find(rd->totlen);
	if (i != cache->end())
	{
		for (unsigned int j=0;j!=i->second.rows.size();j++)
		{
			row & cand = i->second.rows[j];
			if (cand.totlen == rd->totlen && cand.start == cand.start && cand.data == rd->data)
			{
				std::cout << "Match on " << j << "/" << i->second.rows.size() << std::endl;
				cand.uses++;
				return;
			}
		}
		
		i->second.rows.push_back(*rd);
		std::cout << "Non match, growing to " << i->second.rows.size() << " entries" << std::endl;
		return;
	}
	else
	{
		row_length r;
		cache->insert(std::make_pair(rd->totlen, r));
	}
}

void row_cache_print(row_cache *cache)
{
	row_cache::iterator i = cache->begin();
	while (i != cache->end())
	{
		for (int j=0;j!=i->second.rows.size();j++)
		{
			const row & r = i->second.rows[j];
			if (r.uses > 1)
				std::cout << "length " << i->first << " uses=" << r.uses << std::endl;
			int k = 0;
			std::cout << "printing " << i->first << " [" << j << "] rtart=" << (int)r.start << " data=" << r.data.size() << std::endl;
			while (k != r.start)
			{
				std::cout << ".";
				k++;
			}
			
			const int end = r.start + r.data.size();
			while (k != end)
			{
				if (r.data[k - r.start])
					std::cout << (int)(0 + ((unsigned char)r.data[k - r.start])/26);
				else
					std::cout << ".";
				k++;
			}
			
			std::cout << "   outputed " << k << std::endl;
			/*
			while (k != i->first)
			{
				std::cout << ".";
				k++;
			}
			*/
			std::cout << std::endl;
		}
		i++;
	}
}

void row_cache_add(row_cache *cache, char *data, int length)
{
	// figure out start
	int start = 0, end = length - 1;
	for (;start<length && !data[start];start++);
	for (;end>=0 && !data[end];end--);
	
	end++;
		
	if (end <= start)
	{
		std::cout << "Blank row." << std::endl;
		return;
	}
	
	if (start > 255) start = 255;

	row r;
	r.start = 0;
	r.totlen = end;
	r.uses = 1;

	std::cout << "Row[" << length << "] start=" << start << " end=" << end << " totlen=" << r.totlen << std::endl;
	
	for (int i=0;i<end;i++)
		r.data.push_back(data[i]);
		
	row_cache_insert(cache, &r);
}
