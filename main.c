/* See LICENSE file for copyright and license details */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arg.h"
#include "gen.h"
#include "log.h"
#include "vm16.h"
#include "zone.h"

char const *argv0;

char *usage = "[-h] [-d] [-i <inpath>] [-o <outpath>] [file]\n";

static long
flen(FILE *fp)
{
	long size;

	if (0 > fseek(fp, 0, SEEK_END)) {
		return -1;
	}
	size = ftell(fp);
	if (0 > size) {
		return -1;
	}
	rewind(fp);

	return size;
}

static char *
strff(FILE *fp)
{
	size_t len;
	char *buf;

	len = flen(fp);
	buf = zone_alloc((len + 1));
	fread(buf, 1, len, fp);
	buf[len] = '\0';
	return buf;
}

int
main(int argc, char **argv)
{
	(void)argc;
	char *inpath = NULL;
	char *outpath = NULL;
	char *runpath = NULL;
	bool dump = false;

	argv0 = argv[0];
	argv += 1;

	//printf("%x\n", vm16_orri(VM16_JALR, 0, 4, 0x3f));
	//printf("%x\n", vm16_orrar(VM16_MATH, 4, 5, VM16_SLL, 6));

	ARG_BEGIN(argv) {
	case 'h':
		log_info("vm16 %s", usage);
		exit(0);
	case 'i':
		inpath = ARGP(argv);
		if (!inpath) {
			log_fatal("No input file provided for -i\n");
		}
		break;
	case 'o':
		outpath = ARGP(argv);
		if (!outpath) {
			log_fatal("No output file provided for -o\n");
		}
		break;
	case 'd':
		dump = true;
		continue;
	case '-':
		ARGT(argv);
		break;
	default:
		log_fatal("Invalid option '-%c', try '%s -h'\n", ARGF(argv), argv0);
	} ARG_END

	runpath = argv[0];

	if (!outpath) {
		outpath = "a.out";
	}

	if (inpath) {
		FILE *fp;
		struct txt in;
		uint16_t out[VM16_MM_SIZE];
		struct vm16 *v = malloc(sizeof(*v));
		size_t nwords;

		fp = fopen(inpath, "ro");
		if (!fp) {
			log_fatal("Unable to open '%s'\n", inpath);
		}

		txt_init(&in, inpath, strff(fp));
		fclose(fp);

		nwords = assemble(&in, out);

		vm16_init(v);
		vm16_load(v, out, nwords);

		printf("==== begin program ====\n");
		for (int i = 0; i < 32; ++i)
			printf("0x%x\n", out[i]);
		printf("==== end program ====\n");

		if (dump) {
			while (v->pc != VM16_ADDR_HALT) {
				vm16_dump(stdout, v);
				vm16_step(v);
				sleep(1);
			}
		} else {
			vm16_exec(v);
		}
		free(v);
	}

	if (runpath) {
		/* Now run the assembled program */
	}

	return 0;
}
