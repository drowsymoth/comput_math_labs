#include <array>
#include <cstdio>
#include <cstring>
#include <random>
#include <unordered_set>
#include <vector>
#define MAX_LINE 1024
#define FEATURE_COUNT 4
#define CLUSTER_COUNT 3

struct point {
  std::array<float, FEATURE_COUNT> coord = {};
};

struct cluster {
  std::vector<point> points = {};
  point centre;
};

float squared_dist(const point &a, const point &b) {
  float result = 0;
  for (int i = 0; i < FEATURE_COUNT; i++) {
    float di = a.coord[i] - b.coord[i];
    result += di * di;
  }
  return result;
}

cluster *clust_belong(std::array<cluster, CLUSTER_COUNT> &clusters,
                      const point &a) {
  cluster *clust = &clusters[0];
  float nearest = squared_dist(clust->centre, a);
  for (cluster &i : clusters) {
    float temp = squared_dist(i.centre, a);
    if (temp < nearest) {
      nearest = temp;
      clust = &i;
    }
  }
  return clust;
}

point find_centre(const cluster &clust) {
  if (!clust.points.size())
    return {};
  point centre = {};
  for (point i : clust.points) {
    for (int j = 0; j < i.coord.size(); j++) {
      centre.coord[j] += i.coord[j];
    }
  }
  for (float &i : centre.coord) {
    i /= clust.points.size();
  }
  return centre;
}

float clust_centre_dif(const std::array<cluster, CLUSTER_COUNT> &a,
                       const std::array<cluster, CLUSTER_COUNT> &b) {
  float result = 0;
  for (int i = 0; i < CLUSTER_COUNT; i++) {
    for (int j = 0; j < FEATURE_COUNT; j++) {
      result += std::fabs(a[i].centre.coord[j] - b[i].centre.coord[j]);
    }
  }
  return result / (CLUSTER_COUNT * FEATURE_COUNT);
}

std::array<cluster, CLUSTER_COUNT> clustering(std::vector<point> &data,
                                              float E) {

  if (data.size() < CLUSTER_COUNT)
    return {};

  std::array<cluster, CLUSTER_COUNT> current;

  std::mt19937 rng(1234);
  std::uniform_int_distribution<int> dist_int(0, data.size() - 1);

  std::unordered_set<int> chosen;
  int r = 0;

  for (int i = 0; i < current.size(); i++) {
    do {
      r = dist_int(rng);
    } while (chosen.find(r) != chosen.end());
    chosen.insert(r);
    current[i].centre = data[r];
  }

  for (point i : data) {
    clust_belong(current, i)->points.push_back(i);
  }

  std::array<cluster, CLUSTER_COUNT> prev_clust_coord = {};
  do {
    prev_clust_coord = current;
    for (int i = 0; i < CLUSTER_COUNT; i++) {
      current[i].centre = find_centre(current[i]);
    }
    for (cluster &i : current) {
      i.points.clear();
    }
    for (point i : data) {
      clust_belong(current, i)->points.push_back(i);
    }
    // printf("%f\n", clust_centre_dif(prev_clust_coord, current));
  } while (clust_centre_dif(prev_clust_coord, current) > E);

  return current;
}

void print_point(const point &a) {
  for (int i = 0; i < FEATURE_COUNT; i++) {
    printf("\t%f", a.coord[i]);
  }
  printf("\n");
}

void print_cluster(const std::array<cluster, CLUSTER_COUNT> a) {
  for (int i = 0; i < CLUSTER_COUNT; i++) {
    printf("%d cluster countains points:\n", i);
    for (int j = 0; j < a[i].points.size(); j++) {
      print_point(a[i].points[j]);
    }
  }
}

std::vector<point> read_csv_to_points(char *path) {
  std::vector<point> points;
  FILE *fp = fopen(path, "r");
  if (!fp) {
    printf("File reading error\n");
    return {};
  }

  char line[MAX_LINE];

  while (fgets(line, sizeof(line), fp)) {
    char *token = strtok(line, ",");
    point temp = {};
    int count = 0;
    while (token && count < temp.coord.size()) {
      temp.coord[count++] = atof(token);
      token = strtok(NULL, ",");
    }
    points.push_back(temp);
  }
  fclose(fp);
  return points;
}

void write_centres_to_file(std::array<cluster, CLUSTER_COUNT> &a) {
  FILE *fp = fopen("results.csv", "w");
  char line[MAX_LINE] = "";
  for (int i = 0; i < a.size(); i++) {
    for (int j = 0; j < FEATURE_COUNT; j++) {
      char buf[50];
      sprintf(buf, "%f,", a[i].centre.coord[j]);
      strcat(line, buf);
    }
    *strrchr(line, ',') = '\0';
    strcat(line, "\n");
  }
  fprintf(fp, "%s", line);
  fclose(fp);
}

int main() {
  std::vector<point> data = read_csv_to_points("data/Iris_copy.csv");
  std::array<cluster, CLUSTER_COUNT> clusters = clustering(data, 0.1);
  print_cluster(clusters);
  write_centres_to_file(clusters);
  return 0;
}
