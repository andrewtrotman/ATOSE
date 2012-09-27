/*
	COMPUTE_BCH_BITS.C
	------------------
*/
#include <stdio.h>

/*
	MAIN()
	------
*/
int main(void)
{
long long spare_bytes_per_sector, bytes_per_sector, bytes_per_BCH_subsector;
double ecc;
long ecc_long;

while (1)
	{
	printf("bytes_per_sector:");
	fflush(stdout);
	scanf("%lld", &bytes_per_sector);

	printf("spare_bytes_per_sector:");
	fflush(stdout);
	scanf("%lld", &spare_bytes_per_sector);

	printf("bytes_per_BCH_subsector:");
	fflush(stdout);
	scanf("%lld", &bytes_per_BCH_subsector);

	puts("");
	printf("bytes_per_sector       :%lld\n", bytes_per_sector);
	printf("spare_bytes_per_sector :%lld\n", spare_bytes_per_sector);
	printf("bytes_per_BCH_subsector:%lld\n", bytes_per_BCH_subsector);
	ecc = (((double)spare_bytes_per_sector * 8.0) / (((double)bytes_per_sector / (double)bytes_per_BCH_subsector) * 13.0));
	printf("BEST BCH EEC BITS      :%f\n", ecc);

	ecc_long = ((spare_bytes_per_sector * 8) / ((bytes_per_sector / bytes_per_BCH_subsector) * 13));
	printf("Tell the i.MX233 BCH   :%lld\n\n", (ecc_long >> 1) << 1);
	}

return 0;
}
