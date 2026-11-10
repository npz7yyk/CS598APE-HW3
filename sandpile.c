#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static void die(const char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int N = 801;
int W;
int *grid;
#define G(i, j) grid[((i) + 1) * W + (j) + 1]

// you can also use this function to debug print your sandpile
static void render(char *filename) {
  int chan = 3;
  unsigned char *pixels = (unsigned char *)malloc(N * N * chan * sizeof(char));
  int stride = N * chan;
  if (!pixels)
    die("malloc failed for PNG pixel buffer");
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      unsigned char r = 0, g = 0, b = 0;
      switch (G(i, j)) {
      case 0: // obsidian
        r = 15;
        g = 15;
        b = 25;
        break;
      case 1: // electric cyan
        r = 0;
        g = 210;
        b = 255;
        break;
      case 2: // vibrant magenta
        r = 230;
        g = 30;
        b = 130;
        break;
      case 3: // bright gold
        r = 255;
        g = 200;
        b = 0;
        break;
      default: // white
        r = 0;
        g = 0;
        b = 0;
      }

      int pid = (i * N + j) * chan;
      pixels[pid] = r;
      pixels[pid + 1] = g;
      pixels[pid + 2] = b;
    }
  }
  if (stbi_write_png(filename, N, N, chan, pixels, stride)) {
    printf("Successfully saved sandpile identity to: %s\n", filename);
  } else {
    die("Error: stb_image_write failed to save png file\n");
  }
  free(pixels);
}

static int *lo, *hi;

static uint8_t *cur, *nxt, *msk;
static int *ua, *ub;
static int *sinks, nsinks;
static int *srow;
static long *cum;
#define FLUSH 32
static int sym, rows;

static void setup() {
  lo = (int *)malloc(N * sizeof(int));
  hi = (int *)malloc(N * sizeof(int));
  for (int i = 0; i < N; ++i) {
    lo[i] = 0;
    hi[i] = -1;
    int first = 1;
    for (int j = 0; j <= MAX(i, N - 1 - i); ++j) {
      if ((i - N / 2) * (i - N / 2) + (j - N / 2) * (j - N / 2) <=
          (N / 2) * (N / 2)) {
        if (first)
          lo[i] = j;
        first = 0;
        hi[i] = j;
      }
    }
  }

  size_t cells = (size_t)(N + 2) * W;
  cur = (uint8_t *)calloc(cells, 1);
  nxt = (uint8_t *)calloc(cells, 1);
  msk = (uint8_t *)calloc(cells, 1);
  ua = (int *)malloc((N + 2) * sizeof(int));
  ub = (int *)malloc((N + 2) * sizeof(int));
  sinks = (int *)malloc(cells * sizeof(int));
  srow = (int *)malloc((N + 3) * sizeof(int));
  cum = (long *)malloc((N + 3) * sizeof(long));
  if (!cur || !nxt || !msk || !ua || !ub || !sinks || !srow || !cum)
    die("malloc failed for the stabilizer");
  for (int i = 0; i < N; ++i)
    for (int j = lo[i]; j <= hi[i]; ++j)
      msk[(i + 1) * W + j + 1] = 0xFF;
  sym = N % 2;
  rows = sym ? N / 2 + 1 : N;
  nsinks = 0;
  cum[1] = 0;
  for (int r = 1; r <= rows; ++r) {
    srow[r] = nsinks;
    int a = W, b = -1;
    for (int k = r - 1; k <= r + 1; ++k) {
      int i = k - 1;
      if (i < 0 || i >= N || lo[i] > hi[i])
        continue;
      a = MIN(a, lo[i]);
      b = MAX(b, hi[i] + 2);
    }
    ua[r] = a;
    ub[r] = b;
    for (int j = a; j <= b; ++j)
      if (!msk[r * W + j])
        sinks[nsinks++] = r * W + j;
    cum[r + 1] = cum[r] + MAX(0, b - a + 1);
  }
  srow[rows + 1] = nsinks;
}

static void flush(uint8_t *v, int r0, int r1) {
  for (int k = srow[r0]; k < srow[r1]; ++k) {
    grid[sinks[k]] += v[sinks[k]];
    v[sinks[k]] = 0;
  }
}

static inline uint8_t update_row(const uint8_t *cur, uint8_t *nxt, int r) {
  int mid = sym && r == rows;
  const uint8_t *restrict c = cur + (size_t)r * W;
  const uint8_t *restrict u = c - W, *restrict d = mid ? u : c + W;
  const uint8_t *restrict m = msk + (size_t)r * W;
  const uint8_t *restrict mu = m - W, *restrict md = mid ? mu : m + W;
  uint8_t *restrict n = nxt + (size_t)r * W;
  int a = ua[r], b = ub[r];
  uint8_t any = 0;
  for (int j = a; j <= b; ++j) {
    uint8_t q = (c[j] >> 2) & m[j];
    n[j] = (uint8_t)(c[j] - (q << 2) + ((u[j] >> 2) & mu[j]) +
                     ((d[j] >> 2) & md[j]) + ((c[j - 1] >> 2) & m[j - 1]) +
                     ((c[j + 1] >> 2) & m[j + 1]));
    any |= q;
  }
  return any;
}

static int first_row(int t, int T) {
  long goal = cum[rows + 1] * t / T;
  int r = 1;
  while (r <= rows && cum[r] < goal)
    ++r;
  return r;
}

static void stabilize() {
  for (int r = 1; r <= rows; ++r) {
    for (int j = ua[r]; j <= ub[r]; ++j) {
      size_t x = (size_t)r * W + j;
      if (msk[x]) {
        if (grid[x] < 0 || grid[x] > 7)
          die("stabilize() needs 0 to 7 grains per cell");
        cur[x] = (uint8_t)grid[x];
      } else {
        cur[x] = 0;
      }
    }
  }
  int T = omp_get_max_threads();
  uint8_t *post = (uint8_t *)calloc((size_t)2 * T * 64, 1);
  if (!post)
    die("malloc failed for the step flags");
#pragma omp parallel num_threads(T)
  {
    int t = omp_get_thread_num(), nt = omp_get_num_threads();
    int r0 = first_row(t, nt), r1 = first_row(t + 1, nt);
    uint8_t *c = cur, *n = nxt;
    for (long steps = 1;; ++steps) {
      uint8_t any = 0;
      for (int r = r0; r < r1; ++r)
        any |= update_row(c, n, r);
      if (steps % FLUSH == 0)
        flush(n, r0, r1);
      uint8_t *slot = post + (steps % 2) * T * 64;
      slot[t * 64] = any;
#pragma omp barrier
      any = 0;
      for (int k = 0; k < nt; ++k)
        any |= slot[k * 64];
      uint8_t *x = c;
      c = n;
      n = x;
      if (!any)
        break;
    }
    flush(c, r0, r1);
    if (t == 0) {
      cur = c;
      nxt = n;
    }
  }
  free(post);
  for (int i = 0; i < rows; ++i)
    for (int j = lo[i]; j <= hi[i]; ++j)
      G(i, j) = cur[(i + 1) * W + j + 1];
  if (sym)
    for (int i = N / 2 + 1; i < N; ++i)
      memcpy(&G(i, -1), &G(N - 1 - i, -1), W * sizeof(int));
}

void find_identity() {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      G(i, j) = 6;
    }
  }
  stabilize();
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      G(i, j) = 6 - G(i, j);
    }
  }
  stabilize();
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 2) {
    fprintf(stderr, "Usage: %s N\n", argv[0]);
    return EXIT_FAILURE;
  }
  char *end;
  uintmax_t parsed = strtoumax(argv[1], &end, 10);
  if (errno || argv[1][0] < '0' || argv[1][0] > '9' || *end || !parsed ||
      parsed > INT32_MAX)
    die("grid size must be a positive int");
  N = (int)parsed;

  W = (N + 3 + 63) / 64 * 64;
  grid = (int *)calloc((size_t)(N + 2) * W, sizeof(int));
  if (!grid)
    die("malloc failed for the grid");

  setup();
  find_identity();

  render("identity.png");
  free(grid);
  free(lo);
  free(hi);
  free(cur);
  free(nxt);
  free(msk);
  free(ua);
  free(ub);
  free(sinks);
  free(srow);
  free(cum);
  return EXIT_SUCCESS;
}
