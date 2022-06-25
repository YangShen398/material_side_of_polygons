#include "GLWidget.h"

void GLWidget::clear() {
  brect_initialized_ = false;
  point_data_.clear();
  segment_data_.clear();
  vd_.clear();

  polygon_data_.clear();
  disjoint_idx_.clear();
  combined_polygon_set_.clear();
  ordered_pairs_.clear();
}

void GLWidget::read_data(const QString& file_path) {
  QFile data(file_path);
  if (!data.open(QFile::ReadOnly)) {
    QMessageBox::warning(
        this, tr("Voronoi Visualizer"),
        tr("Disable to open file ") + file_path);
  }
  QTextStream in_stream(&data);
  std::size_t num_points, num_segments;
  int x1, y1, x2, y2;
  in_stream >> num_points;
  for (std::size_t i = 0; i < num_points; ++i) {
    in_stream >> x1 >> y1;
    point_type p(x1, y1);
    update_brect(p);
    point_data_.push_back(p);
  }
  in_stream >> num_segments;
  point_type first;
  bool isFirst = true;
  for (std::size_t i = 0; i < num_segments; ++i) {
    in_stream >> x1 >> y1 >> x2 >> y2;
    point_type lp(x1, y1);
    point_type hp(x2, y2);
    if (isFirst)
    {
        first = lp;
        isFirst = false;
    }
    if (first == hp)
    {
        disjoint_idx_.emplace_back(i);
        isFirst = true;
    }
    update_brect(lp);
    update_brect(hp);
    segment_data_.push_back(segment_type(lp, hp));
  }
  in_stream.flush();
}

void GLWidget::update_brect(const point_type& point) {
  if (brect_initialized_) {
    encompass(brect_, point);
  } else {
    set_points(brect_, point, point);
    brect_initialized_ = true;
  }
}

void GLWidget::construct_brect() {
  double side = (std::max)(xh(brect_) - xl(brect_), yh(brect_) - yl(brect_));
  center(shift_, brect_);
  set_points(brect_, shift_, shift_);
  bloat(brect_, side * 1.2);
}

void GLWidget::color_exterior(const VD::edge_type* edge) {
  if (edge->color() == EXTERNAL_COLOR) {
    return;
  }
  edge->color(EXTERNAL_COLOR);
  edge->twin()->color(EXTERNAL_COLOR);
  const VD::vertex_type* v = edge->vertex1();
  if (v == NULL || !edge->is_primary()) {
    return;
  }
  v->color(EXTERNAL_COLOR);
  const VD::edge_type* e = v->incident_edge();
  do {
    color_exterior(e);
    e = e->rot_next();
  } while (e != v->incident_edge());
}

void GLWidget::update_view_port() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  rect_type view_rect = brect_;
  deconvolve(view_rect, shift_);
  glOrtho(xl(view_rect), xh(view_rect),
          yl(view_rect), yh(view_rect),
          -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
}

void GLWidget::draw_points() {
  // Draw input points and endpoints of the input segments.
  glColor3f(0.0f, 0.5f, 1.0f);
  glPointSize(9);
  glBegin(GL_POINTS);
  for (std::size_t i = 0; i < point_data_.size(); ++i) {
    point_type point = point_data_[i];
    deconvolve(point, shift_);
    glVertex2f(point.x(), point.y());
  }
  for (std::size_t i = 0; i < segment_data_.size(); ++i) {
    point_type lp = low(segment_data_[i]);
    lp = deconvolve(lp, shift_);
    glVertex2f(lp.x(), lp.y());
    point_type hp = high(segment_data_[i]);
    hp = deconvolve(hp, shift_);
    glVertex2f(hp.x(), hp.y());
  }
  glEnd();
}

void GLWidget::draw_segments() {
  // Draw input segments.
  glColor3f(0.0f, 0.5f, 1.0f);
  glLineWidth(2.7f);
  glBegin(GL_LINES);
  for (std::size_t i = 0; i < segment_data_.size(); ++i) {
    point_type lp = low(segment_data_[i]);
    lp = deconvolve(lp, shift_);
    glVertex2f(lp.x(), lp.y());
    point_type hp = high(segment_data_[i]);
    hp = deconvolve(hp, shift_);
    glVertex2f(hp.x(), hp.y());
  }
  glEnd();
}

void GLWidget::draw_vertices() {


  glColor3f(0.0f, 0.0f, 0.0f);
  glPointSize(6);
  glBegin(GL_POINTS);
  coordinate_type width = xh(brect_) - xl(brect_);
  double xxhh = xh(brect_);
  double xxll = xl(brect_);
  int xN = 100;
  coordinate_type height = yh(brect_) - yl(brect_);
  double yyhh = yh(brect_);
  double yyll = yl(brect_);
  int yN = 100;
  for (int i = 0; i < xN; ++i)
  {
      for (int j = 0; j < yN; ++j)
      {
          double x = xl(brect_) + (width / xN) * i;
          double y = yl(brect_) + (height / yN) * j;
          point_type vertex(x, y);
          for (const auto& polygon : combined_polygon_set_)
          {
              if (contains(polygon, vertex))
              {
                  vertex = deconvolve(vertex, shift_);
                  glVertex2f(vertex.x(), vertex.y());
              }
          }
      }
  }
  glEnd();

  // Draw voronoi vertices.
//  glColor3f(0.0f, 0.0f, 0.0f);
//  glPointSize(6);
//  glBegin(GL_POINTS);
//  for (const_vertex_iterator it = vd_.vertices().begin();
//       it != vd_.vertices().end(); ++it) {
//    if (internal_edges_only_ && (it->color() == EXTERNAL_COLOR)) {
//      continue;
//    }
//    point_type vertex(it->x(), it->y());
//    vertex = deconvolve(vertex, shift_);
//    glVertex2f(vertex.x(), vertex.y());
//  }
//  glEnd();
}
void GLWidget::draw_edges() {
  // Draw voronoi edges.
//  glColor3f(0.0f, 0.0f, 0.0f);
//  glLineWidth(1.7f);
//  for (const_edge_iterator it = vd_.edges().begin();
//       it != vd_.edges().end(); ++it) {
//    if (primary_edges_only_ && !it->is_primary()) {
//      continue;
//    }
//    if (internal_edges_only_ && (it->color() == EXTERNAL_COLOR)) {
//      continue;
//    }

//    std::vector<point_type> samples;
//    if (!it->is_finite()) {
//      clip_infinite_edge(*it, &samples);
//    } else {
//      point_type vertex0(it->vertex0()->x(), it->vertex0()->y());
//      samples.push_back(vertex0);
//      point_type vertex1(it->vertex1()->x(), it->vertex1()->y());
//      samples.push_back(vertex1);
//      if (it->is_curved()) {
//        sample_curved_edge(*it, &samples);
//      }
//    }
//    glBegin(GL_LINE_STRIP);
//    for (std::size_t i = 0; i < samples.size(); ++i) {
//      point_type vertex = deconvolve(samples[i], shift_);
//      glVertex2f(vertex.x(), vertex.y());
//    }
//    glEnd();
//  }
}

void GLWidget::clip_infinite_edge(
    const edge_type& edge, std::vector<point_type>* clipped_edge) {
  const cell_type& cell1 = *edge.cell();
  const cell_type& cell2 = *edge.twin()->cell();
  point_type origin, direction;
  // Infinite edges could not be created by two segment sites.
  if (cell1.contains_point() && cell2.contains_point()) {
    point_type p1 = retrieve_point(cell1);
    point_type p2 = retrieve_point(cell2);
    origin.x((p1.x() + p2.x()) * 0.5);
    origin.y((p1.y() + p2.y()) * 0.5);
    direction.x(p1.y() - p2.y());
    direction.y(p2.x() - p1.x());
  } else {
    origin = cell1.contains_segment() ?
        retrieve_point(cell2) :
        retrieve_point(cell1);
    segment_type segment = cell1.contains_segment() ?
        retrieve_segment(cell1) :
        retrieve_segment(cell2);
    coordinate_type dx = high(segment).x() - low(segment).x();
    coordinate_type dy = high(segment).y() - low(segment).y();
    if ((low(segment) == origin) ^ cell1.contains_point()) {
      direction.x(dy);
      direction.y(-dx);
    } else {
      direction.x(-dy);
      direction.y(dx);
    }
  }
  coordinate_type side = xh(brect_) - xl(brect_);
  coordinate_type koef =
      side / (std::max)(fabs(direction.x()), fabs(direction.y()));
  if (edge.vertex0() == NULL) {
    clipped_edge->push_back(point_type(
        origin.x() - direction.x() * koef,
        origin.y() - direction.y() * koef));
  } else {
    clipped_edge->push_back(
        point_type(edge.vertex0()->x(), edge.vertex0()->y()));
  }
  if (edge.vertex1() == NULL) {
    clipped_edge->push_back(point_type(
        origin.x() + direction.x() * koef,
        origin.y() + direction.y() * koef));
  } else {
    clipped_edge->push_back(
        point_type(edge.vertex1()->x(), edge.vertex1()->y()));
  }
}

void GLWidget::sample_curved_edge(
    const edge_type& edge,
    std::vector<point_type>* sampled_edge) {
  coordinate_type max_dist = 1E-3 * (xh(brect_) - xl(brect_));
  point_type point = edge.cell()->contains_point() ?
      retrieve_point(*edge.cell()) :
      retrieve_point(*edge.twin()->cell());
  segment_type segment = edge.cell()->contains_point() ?
      retrieve_segment(*edge.twin()->cell()) :
      retrieve_segment(*edge.cell());
  voronoi_visual_utils<coordinate_type>::discretize(
      point, segment, max_dist, sampled_edge);
}

point_type GLWidget::retrieve_point(const cell_type& cell) {
  source_index_type index = cell.source_index();
  source_category_type category = cell.source_category();
  if (category == SOURCE_CATEGORY_SINGLE_POINT) {
    return point_data_[index];
  }
  index -= point_data_.size();
  if (category == SOURCE_CATEGORY_SEGMENT_START_POINT) {
    return low(segment_data_[index]);
  } else {
    return high(segment_data_[index]);
  }
}

segment_type GLWidget::retrieve_segment(const cell_type& cell) {
  source_index_type index = cell.source_index() - point_data_.size();
  return segment_data_[index];
}



void GLWidget::initializeGL() {
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_POINT_SMOOTH);
}

void GLWidget::paintGL() {
  qglClearColor(QColor::fromRgb(255, 255, 255));
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_points();
  draw_segments();
  draw_vertices();
  draw_edges();
}

void GLWidget::resizeGL(int width, int height) {
  int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::timerEvent(QTimerEvent* e) {
  update();
}



void GLWidget::build(const QString& file_path) {
  // Clear all containers.
  clear();

  // Read data.
  read_data(file_path);

  // No data, don't proceed.
  if (!brect_initialized_) {
    return;
  }

  // Construct bounding rectangle.
  construct_brect();

  // Construct voronoi diagram.
  construct_voronoi(
      point_data_.begin(), point_data_.end(),
      segment_data_.begin(), segment_data_.end(),
      &vd_);

  // Color exterior edges.
  for (const_edge_iterator it = vd_.edges().begin();
       it != vd_.edges().end(); ++it) {
    if (!it->is_finite()) {
      color_exterior(&(*it));
    }
  }

  // build disjoint polygon sets
  int pre = 0;
  for (const auto& i : disjoint_idx_)
  {
      std::vector<point_type> pts;
      poly_type polygon;
      for (int j = pre; j <= i; ++j)
      {
          pts.push_back(segment_data_.at(j).low());
      }
      pre = i + 1;
      set_points(polygon, pts.begin(), pts.end());
      polygon_data_.push_back(polygon);
  }


  // build the matrix to depict the relationship between each disjoint polygon
  int rows = polygon_data_.size();
  int columns = polygon_data_.size();
  int* joint_matrix = new int[rows * columns];
  for (int i = 0; i < rows; i++)
  {
      int* row = &joint_matrix[i * columns];

      for (int j = 0; j < columns; j++)
      {
          int* e_ij = &row[j];
          int* col = &joint_matrix[j * rows];
          int* e_ji = &col[i];
          if (i == j)
          {
              (*e_ij) = 0;
              continue;
          }
          // is j-th polygon inside current i-th polygon?
          // if yes, e_ij = 1, e_ji = -1, otherwise, 0
          // we only need to see if one point from polygon(j) is inside polygon(i) or not
          // this is because we are assuming they are disjoint polygons which do not intersect
          if ((*e_ij) == -1)
          {
              continue;
          }
          bool inside = contains(polygon_data_.at(i), polygon_data_.at(j).coords_.at(0));
          (*e_ij) = inside ? 1 : 0;
          (*e_ji) = inside ? -1 : 0;
      }
  }

  // alwasy reserve memory if size is known. This can avoid deallocate and allocate memory inside std::vector to improve performance
  ordered_pairs_.resize(rows);
  // now we fill in ordered_pairs
  int polygon_idx = 0;
  for (auto& pair : ordered_pairs_)
  {
      pair.first = polygon_idx++; // first = index of polygon_data_
      int* row = &joint_matrix[pair.first * columns];
      int num = 0;
      int sign = 1;
      for (int j = 0; j < columns; j++)
      {
          int* e_ij = &row[j];
          num += ((*e_ij) == -1) ? 0 : (*e_ij);
          sign *= ((*e_ij) == 0) ? 1 : (*e_ij);
      }
      pair.second.first = sign; // indicates whether the polygon is inner contour or outer contour, 1=out, -1=in
      pair.second.second = num; // number of polygons encapsulated
  }
  // up to this point ordered_pairs should be initialized with correct values, but not sorted
  // now we need to sort it based on the number of polygons encapsulated
  std::sort(ordered_pairs_.begin(), ordered_pairs_.end(),
      [](std::pair<int, std::pair<int ,int>>& a, std::pair<int, std::pair<int ,int>>& b) { return a.second.second > b.second.second;});
  // perform boolean operations to generate the final combined polygon set
  for (const auto& pair : ordered_pairs_)
  {
      if (pair.second.first == 1)
      {
          combined_polygon_set_ += polygon_data_.at(pair.first);
      }
      else
      {
          combined_polygon_set_ -= polygon_data_.at(pair.first);
      }
  }

  delete [] joint_matrix;
  // Update view port.
  update_view_port();
}

void GLWidget::show_primary_edges_only() {
  primary_edges_only_ ^= true;
}

void GLWidget::show_internal_edges_only() {
  internal_edges_only_ ^= true;
}
