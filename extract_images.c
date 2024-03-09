// https://codeberg.org/caps-universe/libcapsraster/src/tag/libcapsraster-1.0.1/test/test-simple-mupdf.c
// https://mupdf.com/docs/examples/example.c
// sudo apt-get install libmupdf-dev ljbig2dec 
// sudo ln -s /usr/lib/x86_64-linux-gnu/libopenjp2.so.2.3.1 /usr/lib/x86_64-linux-gnu/libopenjp2.so
// sudo ln -s /usr/lib/x86_64-linux-gnu/libjbig2dec.so.0 /usr/lib/x86_64-linux-gnu/libjbig2dec.so
// gcc -o x extract_images.c /usr/lib/libmupdf.a /usr/lib/libmupdf-third.a -lmupdf -lm -lmupdf-third -lfreetype -lharfbuzz -ljpeg -lz -L/usr/lib/x86_64-linux-gnu -lopenjp2 -ljbig2dec
// ./x test.pdf

#include <mupdf/fitz.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	char *input;
	int page_number, page_count;
	fz_context *ctx;
	fz_document *doc;
	fz_pixmap *pix;
	fz_matrix ctm;
	int x, y;

	if (argc < 3)
	{
		fprintf(stderr, "usage: example input-file page-number\n");
		fprintf(stderr, "\tinput-file: path of PDF, XPS, CBZ or EPUB document to open\n");
		fprintf(stderr, "\tPage numbering starts from one.\n");
		return EXIT_FAILURE;
	}

	input = argv[1];
	page_number = atoi(argv[2]) - 1;

	/* Create a context to hold the exception stack and various caches. */
	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx)
	{
		fprintf(stderr, "cannot create mupdf context\n");
		return EXIT_FAILURE;
	}

	/* Register the default file types to handle. */
	fz_try(ctx)
		fz_register_document_handlers(ctx);
	fz_catch(ctx)
	{
		fprintf(stderr, "cannot register document handlers: %s\n", fz_caught_message(ctx));
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	/* Open the document. */
	fz_try(ctx)
		doc = fz_open_document(ctx, input);
	fz_catch(ctx)
	{
		fprintf(stderr, "cannot open document: %s\n", fz_caught_message(ctx));
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	/* Count the number of pages. */
	fz_try(ctx)
		page_count = fz_count_pages(ctx, doc);
	fz_catch(ctx)
	{
		fprintf(stderr, "cannot count number of pages: %s\n", fz_caught_message(ctx));
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	if (page_number < 0 || page_number >= page_count)
	{
		fprintf(stderr, "page number out of range: %d (page count %d)\n", page_number + 1, page_count);
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	ctm = fz_scale(2, 2);

	/* Render page to an RGB pixmap. */
	fz_try(ctx)
		pix = fz_new_pixmap_from_page_number(ctx, doc, page_number, ctm, fz_device_rgb(ctx), 0);
	fz_catch(ctx)
	{
		fprintf(stderr, "cannot render page: %s\n", fz_caught_message(ctx));
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	/* Print image data in ascii PPM format. */
	printf("%d %d %d %li\n", pix->w, pix->h, pix->n, pix->stride);
	printf("P3\n");
	printf("%d %d\n", pix->w, pix->h);
	//printf("255\n");
	//for (y = 0; y < pix->h; ++y)
	//{
	//	unsigned char *p = &pix->samples[y * pix->stride];
	//	for (x = 0; x < pix->w; ++x)
	//	{
	//		if (x > 0)
	//			printf("  ");
	//		printf("%3d %3d %3d", p[0], p[1], p[2]);
	//		p += pix->n;
	//	}
	//	printf("\n");
	//}

	/* Clean up. */
	fz_drop_pixmap(ctx, pix);
	fz_drop_document(ctx, doc);
	fz_drop_context(ctx);
	return EXIT_SUCCESS;
}
