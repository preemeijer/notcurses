#include <string.h>
#include "internal.h"

ncprogbar* ncprogbar_create(ncplane* n, const ncprogbar_options* opts){
  ncprogbar_options default_opts;
  if(opts == NULL){
    memset(&default_opts, 0, sizeof(default_opts));
    default_opts.minchannels = CHANNELS_RGB_INITIALIZER(0x3d, 0x3d, 0x3d, 0, 0, 0);
    default_opts.maxchannels = CHANNELS_RGB_INITIALIZER(0xe0, 0xee, 0xe0, 0, 0, 0);
    opts = &default_opts;
  }
  if(check_gradient_args(opts->minchannels, opts->maxchannels, opts->minchannels, opts->maxchannels)){
    logerror(ncplane_notcurses(n), "Illegal progbar colors\n");
    return NULL;
  }
  if(opts->flags > (NCPROGBAR_OPTION_RETROGRADE << 1u)){
    logwarn(ncplane_notcurses(n), "Invalid flags %016lx\n", opts->flags);
  }
  ncprogbar* ret = malloc(sizeof(*ret));
  ret->ncp = n;
  ret->maxchannels = opts->maxchannels;
  ret->minchannels = opts->minchannels;
  ret->retrograde = opts->flags & NCPROGBAR_OPTION_RETROGRADE;
  return ret;
}

ncplane* ncprogbar_plane(ncprogbar* n){
  return n->ncp;
}

static int
progbar_redraw(ncprogbar* n){
  enum {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT
  } direction;
  // get current dimensions; they might have changed
  int dimy, dimx;
  ncplane_dim_yx(ncprogbar_plane(n), &dimy, &dimx);
  if(dimx > dimy){
    direction = n->retrograde ? DIR_LEFT : DIR_RIGHT;
  }else{
    direction = n->retrograde ? DIR_DOWN : DIR_UP;
  }
  const bool horizontal = (direction == DIR_LEFT || direction == DIR_RIGHT);
  int delt, range;
  if(horizontal){
    range = dimx;
    delt = n->retrograde ? 1 : -1;
  }else{
    range = dimy;
    delt = n->retrograde ? -1 : 1;
  }
  double progress = n->progress * range;
  if(n->retrograde){
    progress = range - progress;
  }
  while(progress > 0 && progress < range){
    // FIXME lerp min->max
    if(ncplane_putegc_yx(ncprogbar_plane(n), 0, progress, "█", NULL) <= 0){
      return -1;
    }
    progress += delt;
  }
  return 0;
}

int ncprogbar_set_progress(ncprogbar* n, double p){
  if(p < 0 || p > 1){
    logerror(ncplane_notcurses(ncprogbar_plane(n)), "Invalid progress %g\n", p);
    return -1;
  }
  n->progress = p;
  return progbar_redraw(n);
}

double ncprogbar_progress(const ncprogbar* n){
  return n->progress;
}

void ncprogbar_destroy(ncprogbar* n){
  ncplane_destroy(n->ncp);
  free(n);
}