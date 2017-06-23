#include <cstdio>
#include <ctime>
using namespace std;

char buf[2048];

void print_ip(const char * str)
{
	for (int i = 0; i < 4; ++i) {
		printf("%u.", (unsigned char)str[i]);
	}
}

int main()
{
	FILE * pf = fopen("../data/equinix-chicago.dirA.20160121-130000.UTC.anon.pcap/equinix-chicago.dirA.20160121-130000.UTC.anon.pcap", "rb");

	fseek(pf, 24, SEEK_CUR);

	FILE * pout = fopen("../data/formatted.dat", "wb");

	int i = 0;
	while (1) {
		unsigned header_buf[4];
		if (!fread(header_buf, 4, 4, pf)) {
			printf("done");
			break;
		}
		
		time_t ut = header_buf[0];
		unsigned cap_len = header_buf[2];

		if (cap_len > 2048) {
			printf("error\n");
			return 0;
		}
		fread(buf, cap_len, 1, pf);

		i++;
		printf("%s(%3d): %u(%u)\n", ctime(&ut), i, cap_len, header_buf[3]);
		print_ip(buf + 12); printf(" -> ");
		print_ip(buf + 16); printf("\n");

		fwrite(header_buf, 4, 2, pout);
		fwrite(buf + 12, 1, 8, pout);
	}

	fclose(pf);
	fclose(pout);

	return 0;
}