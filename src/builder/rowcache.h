#include <vector>
#include <map>

struct row
{
	unsigned int totlen;
	unsigned char start;
	std::vector<char> data;
	unsigned int uses;
	unsigned int comp_data_begin, comp_data_size;
};

struct row_length
{
	std::vector<row> rows;
};

typedef std::map<unsigned int, row_length> row_cache;

// might cut a bit or two of precision here and there.
int lossy_rle_encode(std::vector<char> const & input, unsigned char *output)
{
	int outsize = 0;
	int runlength = 1;
	int old_val = (unsigned char)input[0];

	for (int i=1;i<=input.size();i++)
	{
		unsigned int next = 257; // impossible value
		if (i < input.size())
		    next = (unsigned char)input[i];

		if (next != old_val || runlength > 126)
		{
			// output old run
			if (runlength > 1)
			{
				output[outsize++] = 0x80 | (runlength - 1);
				output[outsize++] = old_val;
			}
			else if (runlength == 1)
			{
				if (old_val == 255)
					old_val = 254;
				output[outsize++] = (unsigned char)(old_val / 2);
			}

			// encode new
			old_val = next;
			runlength = 1;
		}
		else
		{
			// equal value.
			runlength++;
		}
	}

	return outsize;
}

// compress into rle buffer.
void row_cache_compress(row_cache *cache, std::vector<unsigned char> *out)
{
	row_cache::iterator i = cache->begin();
	while (i != cache->end())
	{
		for (int j=0;j!=i->second.rows.size();j++)
		{
			row & r = i->second.rows[j];
			unsigned char encoded[2048];
			r.comp_data_begin = out->size();
			r.comp_data_size = lossy_rle_encode(r.data, &encoded[0]);
			out->insert(out->end(), encoded, encoded + r.comp_data_size);
		}
		i++;
	}
}

// returns row pointer into structure, only valid till next row is inserted.
row* row_cache_insert(row_cache *cache, row *rd)
{
	row_cache::iterator i = cache->find(rd->totlen);
	if (i != cache->end())
	{
		for (unsigned int j=0;j!=i->second.rows.size();j++)
		{
			row & cand = i->second.rows[j];
			if (cand.totlen == rd->totlen && cand.start == rd->start)
			{
				int err = 0;
				for (int k=0;k!=cand.data.size();k++)
				{
					const int diff = (int)cand.data[k] - (int)rd->data[k];
					err += diff * diff;
				}

				if (err > 100)
					continue;

				cand.uses++;
				return &cand;
			}
		}
		
		i->second.rows.push_back(*rd);
		return 0;
	}
	else
	{
		row_length r;
		r.rows.push_back(*rd);
		cache->insert(std::make_pair(rd->totlen, r));
		return 0;
	}
}

void row_cache_print(row_cache *cache)
{
	int uncomp_bytes = 0;
	int comp_bytes = 0;

	row_cache::iterator i = cache->begin();
	while (i != cache->end())
	{
		for (int j=0;j!=i->second.rows.size();j++)
		{
			const row & r = i->second.rows[j];
			if (r.uses < 2)
				continue;

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

			unsigned char encoded[2048];
			int bytes = lossy_rle_encode(r.data, &encoded[0]);

			std::cout << "   outputed " << k <<  " rle=" << bytes << " bytes, uses=" << r.uses << std::endl;

			comp_bytes += bytes;
			uncomp_bytes += r.data.size();

			// rle encode
			std::cout << std::endl;
		}
		i++;
	}
	std::cout << "Raw image data is " << uncomp_bytes << " bytes, rle compressed is " << comp_bytes << std::endl;
}

row* row_cache_add(row_cache *cache, char *data, int length)
{
	// figure out start
	int start = 0, end = length - 1;
	for (;start<length && !data[start];start++);
	for (;end>=0 && !data[end];end--);
	
	end++;
		
	if (end <= start)
		return 0;

	if (start > 255)
		start = 255;

	row r;
	r.start = 0;
	r.totlen = end;
	r.uses = 1;
	r.comp_data_size = 0;
	r.comp_data_begin = ~0;

	for (int i=0;i<end;i++)
		r.data.push_back(data[i]);
		
	return row_cache_insert(cache, &r);
}
