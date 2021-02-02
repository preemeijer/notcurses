#include <locale.h>
#include <notcurses/notcurses.h>
#include <stdlib.h>

static struct ncplane *make_status_line_plane(struct ncplane *std_plane) {
  struct ncplane_options opts = {0};
  opts.rows = 1;
  opts.cols = ncplane_dim_x(std_plane);
  opts.y = ncplane_dim_y(std_plane) - 1;
  opts.x = 0;

  nccell c = CELL_CHAR_INITIALIZER(' ');
  cell_set_bg_rgb8(&c, 0x20, 0x20, 0x20);

  struct ncplane *status_line_plane = ncplane_create(std_plane, &opts);
  ncplane_set_base_cell(status_line_plane, &c);
  return status_line_plane;
}

static void print_status_line(struct ncplane *plane) {
  ncplane_putstr_aligned(plane, 0, NCALIGN_LEFT, " ?: help");
}

static void print_help(struct ncplane *plane) {
  const int width = ncplane_dim_x(plane);
  ncplane_erase(plane);
  ncplane_putstr_aligned(
      plane, 0, NCALIGN_LEFT,
      " j/k: scroll up/down   C-B/C-F: page up/down   q: quit");
  ncplane_putstr_aligned(plane, 0, NCALIGN_RIGHT,
                         "press any key to close help ");
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  notcurses_options opts = {0};
  opts.flags = NCOPTION_SUPPRESS_BANNERS;

  struct notcurses *nc;
  if ((nc = notcurses_core_init(&opts, NULL)) == NULL) {
    return EXIT_FAILURE;
  }

  struct ncplane *std_plane = notcurses_stdplane(nc);
  struct ncplane *status_line_plane = make_status_line_plane(std_plane);

  ncplane_move_top(status_line_plane);

  struct ncinput input = {0};
  bool quit = false;
  while (!quit) {
    print_status_line(status_line_plane);
    notcurses_render(nc);
    notcurses_getc_blocking(nc, &input);
    switch (input.id) {
    case '?':
      print_help(status_line_plane);
      notcurses_render(nc);
      notcurses_getc_blocking(nc, &input);
      ncplane_erase(status_line_plane);
      break;
    case 'q':
      quit = true;
      break;
    }
  }

  notcurses_stop(nc);
}
