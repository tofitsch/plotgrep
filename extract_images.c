// gcc -o x extract_images.c $(pkg-config --cflags --libs poppler-glib)
// ./x file://`realpath test.pdf` t

#include <stdio.h>
#include <poppler/glib/poppler.h>
#include <cairo.h>

int main(int argc, char *argv[]) {

  if (argc < 3) {
    printf("Usage: %s <input_pdf> <output_prefix>\n", argv[0]);
    return 1;
  }

  char *input_pdf = argv[1];
  char *output_prefix = argv[2];

  GError *error = NULL;
  PopplerDocument *document = poppler_document_new_from_file(input_pdf, NULL, &error);

  if (error != NULL) {
      fprintf(stderr, "Error: %s\n", error->message);
      g_error_free(error);
      return 1;
  }

  if (!document) {
      g_printerr("Error opening PDF file: %s\n", error->message);
      g_error_free(error);
      return 1;
  }

  int n_pages = poppler_document_get_n_pages(document);

  for (int i = 0; i < n_pages; ++i) {

      PopplerPage *page = poppler_document_get_page(document, i);

      double width, height;
      poppler_page_get_size(page, &width, &height);

      cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
      cairo_t *cr = cairo_create(surface);

      poppler_page_render(page, cr);
      cairo_surface_write_to_png(surface, g_strdup_printf("%s_%d.png", output_prefix, i + 1));

      cairo_surface_destroy(surface);
      cairo_destroy(cr);
      g_object_unref(page);

  }

  g_object_unref(document);
  return 0;

}
