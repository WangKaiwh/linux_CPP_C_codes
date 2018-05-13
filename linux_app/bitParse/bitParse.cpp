/**
 * Author: Kevin Wang
 * Date: 2018.5.13
 * Free Software
 */
#include <iostream>
#include <vector>
#include <cstdio>
#include <cmath>

using std::vector;

typedef unsigned long u64;
typedef unsigned char u8;

#define SEG_OF_VAL(val, seg_index)  \
		(  ( (val) >> ( (seg_index)*_seg_bit_len) ) \
			& 0xf  )


class bitParse {
public:
	bitParse(u64 val, u8 val_bit_len=32) : _val(val), _seg_bit_len(4)
		, _val_bit_len(val_bit_len)
	{}

	void parse()
	{
		_val_to_segval();

		_print_bit_tag();

		printf("    ");

		for (int i = _val_bit_len/_seg_bit_len - 1;
				i >= 0; i--)
		{
			_print_segval_with_bit(_segment[i], i);
		}
		printf("\n");
	}

private:
	bool _val_to_segval()
	{
		u8 segval = 0;
		for (int i = 0; i < _val_bit_len/_seg_bit_len; i++)
		{
			segval = SEG_OF_VAL(_val, i);
			_segment.push_back(segval);
		}
	}

	void _print_segval_with_bit(u8 segment, u8 seg_index) const
	{
		for (int i = 0; i < _seg_bit_len; i++)
		{
			printf("%01d  ", (segment>>i) & 0x1);
		}
	}

	void _print_bit_tag() const
	{
		printf("bit ");
		for (int i=_val_bit_len-1; i>=0; i--)
		{
			printf("%02d ", i);
		}
		printf("\n");
	}

	u64 _val;
	vector<u8> _segment;
	const int _seg_bit_len;
	int _val_bit_len;
};

int main(int argc, char **argv)
{
	bitParse val(0xaa55);

	val.parse();

	return 0;
}
