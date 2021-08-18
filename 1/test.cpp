#include <DxLib.h>
#include <vector>
#include <deque>
#include <cmath>
using namespace std;

#define PI 3.141592653589793f

typedef struct {
	int x, y, sx, sy, md, movespeed, type, ref;
	float angle;
	vector<vector<int>> coordinate;
}Bullet_t;

typedef struct {
	int x, y, cx, cy, move_speed;
	float angle_low, angle_up;
	vector<vector<int>> coordinate_low;
	vector<vector<int>> coordinate_up;
	vector<vector<int>> coordinate_turret;
	deque<Bullet_t> bullet;
}Ch_t;

long long flame;
const int window_w = 640;
const int window_h = 480;
const int window_w_mid = window_w / 2;
const int window_h_mid = window_h / 2;
bool window_mode = true;
bool input_mode = true;
char keys[256];
Ch_t ch;
const int ch_capacity = 700;
const int tank_size_low = 12;
const int tank_size_up = 6;
const int tank_turret_length = 18;
const int bullet_w = 2;
const int bullet_h = 6;
const int bullet_speed = 5;
const int bullet_ref_limit = 1;
vector<vector<int>> tank_coordinate_low = { {-tank_size_low, -tank_size_low}, {tank_size_low, -tank_size_low}, {tank_size_low, tank_size_low}, {-tank_size_low, tank_size_low} };
vector<vector<int>> tank_coordinate_up = { {-tank_size_up, -tank_size_up}, {tank_size_up, -tank_size_up}, {tank_size_up, tank_size_up}, {-tank_size_up, tank_size_up} };
vector<vector<int>> tank_coordinate_turret = { {-tank_size_up / 3, tank_size_up}, {tank_size_up / 3, tank_size_up}, {tank_size_up / 3, tank_size_up + tank_turret_length},{-tank_size_up / 3, tank_size_up + tank_turret_length} };
vector<vector<int>> bullet_coordinate = { {-bullet_w, -bullet_h}, {bullet_w, -bullet_h}, {0, bullet_h} };
const int ground_w = 600;
const int ground_h = 360;
const int ground_sx = 20;
const int ground_sy = 100;
const vector<vector<int>> ground_mate = { {ground_sx, ground_sy}, {ground_sx + ground_w, ground_sy}, {ground_sx + ground_w, ground_sy + ground_h}, {ground_sx, ground_sy + ground_h} };
vector<vector<vector<int>>> ground = { { {0, 0}, {ground_mate[0][0], 0}, {ground_mate[0][0], window_h}, {0, window_h} }, { {0, 0}, {window_w, 0}, {window_w, ground_mate[0][1]}, {0, ground_mate[0][1]} }, { {ground_mate[1][0], 0}, {window_w, 0},{window_w, window_h}, {ground_mate[1][0], window_h} }, { {0, ground_mate[3][1]}, {window_w, ground_mate[3][1]}, {window_w, window_h}, {0, window_h} } };
vector<vector<vector<int>>> quadrange_all = {};
deque<Bullet_t> bullet_flying = {};
const vector<unsigned int> bw = { GetColor(0, 0, 0), GetColor(255, 255, 255) };
const unsigned int red = GetColor(255, 0, 0);
const unsigned int pink = GetColor(199, 21, 133);
long long ch_last_shot_flame = 0;
bool ch_scope = false;
const long long ch_shot_interval_flame = 10;

float pythagoras(float x1, float y1, float x2, float y2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	return (dx * dx) - (dy * dy);
}

float radian(float t) {
	return t * PI / 180.0f;
}

float degree(float t) {
	return t * 180.0f / PI;
}

vector<vector<int>> format_quadrangle(vector<vector<int>> fvec, float x, float y) {
	vector<vector<int>> formatted(4, vector<int>(2));
	for (int i = 0; i < 4; ++i) {
		float px = fvec[i][0];
		float py = fvec[i][1];
		formatted[i][0] = x + px;
		formatted[i][1] = y + py;
	}
	return formatted;
}

vector<vector<int>> format_triangle(vector<vector<int>> fvec, float x, float y) {
	vector<vector<int>> formatted(3, vector<int>(2));
	for (int i = 0; i < 3; ++i) {
		float px = fvec[i][0];
		float py = fvec[i][1];
		formatted[i][0] = x + px;
		formatted[i][1] = y + py;
	}
	return formatted;
}

vector<vector<int>> roll_quadrangle(vector<vector<int>> cvec, float x, float y, float theta) {
	float t = radian(theta);
	vector<vector<int>> rolled(4, vector<int>(2));
	for (int i = 0; i < 4; ++i) {
		float a = cvec[i][0];
		float b = cvec[i][1];
		rolled[i][0] = x + (a - x) * cos(t) - (b - y) * sin(t);
		rolled[i][1] = y + (a - x) * sin(t) + (b - y) * cos(t);
	}
	return rolled;
}

vector<vector<int>> roll_triangle(vector<vector<int>> cvec, float x, float y, float theta) {
	float t = radian(theta);
	vector<vector<int>> rolled(3, vector<int>(2));
	for (int i = 0; i < 3; ++i) {
		float a = cvec[i][0];
		float b = cvec[i][1];
		rolled[i][0] = x + (a - x) * cos(t) - (b - y) * sin(t);
		rolled[i][1] = y + (a - x) * sin(t) + (b - y) * cos(t);
	}
	return rolled;
}

Bullet_t bullet_generate(int x, int y, int sx, int sy, int md, float angle, int type, int ref, int movespeed) {
	Bullet_t bullet;
	switch (type) {
	case 0:
		bullet.x = x;
		bullet.y = y;
		bullet.sx = sx;
		bullet.sy = sy;
		bullet.md = md;
		bullet.angle = angle;
		bullet.ref = ref;
		bullet.movespeed = movespeed;
		bullet.coordinate = roll_triangle(format_triangle(bullet_coordinate, x, y), x, y, angle);
		break;
	}
	return bullet;
}

void init_ch(int x, int y, int cx, int cy, float angle_low, float angle_up, deque<Bullet_t> bullet) {
	ch.x = x;
	ch.y = y;
	ch.cx = cx;
	ch.cy = cy;
	ch.move_speed = 4;
	ch.angle_low = angle_low;
	ch.angle_up = angle_up;
	ch.coordinate_low = format_quadrangle(tank_coordinate_low, ch.x, ch.y);
	ch.coordinate_up = format_quadrangle(tank_coordinate_up, ch.x, ch.y);
	ch.coordinate_turret = format_quadrangle(tank_coordinate_turret, ch.x, ch.y);
	ch.bullet = bullet;
}

bool check_quadrangle_in_quadrangle(vector<vector<int>> a, vector<vector<int>> b, vector<vector<int>> b2) {
	if (a[0][1] < b[0][1] && a[1][0] > b[1][0] && a[2][1] > b[2][1] && a[0][0] < b[0][0]) {
		for (int i = 0; i < 4; ++i) {
			if (a[0][0] > b2[i][0] || a[1][0] < b2[i][0]) {
				return false;
			}
			else if (a[0][1] > b2[i][1] || a[2][1] < b2[i][1]) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool chech_pixel_in_quadrange(int x, int y, vector<vector<int>> a) {
	if (a[0][0] <= x && x <= a[1][0]) {
		if (a[0][1] <= y && y <= a[3][1]) {
			return true;
		}
	}
	return false;
}

bool check_bullet_in_quadrange(Bullet_t b, vector<vector<int>> a) {
	for (int i = 0; i < 3; ++i) {
		int x = b.coordinate[i][0];
		int y = b.coordinate[i][1];
		if (chech_pixel_in_quadrange(x, y, a)) return false;
	}
	return true;
}

bool check_bullet_in_ground(Bullet_t b) {
	for (vector<vector<int>> g : quadrange_all) {
		for (int i = 0; i < 3; ++i) {
			int x = b.coordinate[i][0];
			int y = b.coordinate[i][1];
			if (chech_pixel_in_quadrange(x, y, g)) return false;
		}
	}
	return true;
}

bool check_cross_line_and_line(vector<vector<int>> a, vector<vector<int>> b) {
	int x1 = a[0][0];
	int y1 = a[0][1];
	int x2 = a[1][0];
	int y2 = a[1][1];
	int x3 = b[0][0];
	int y3 = b[0][1];
	int x4 = b[1][0];
	int y4 = b[1][1];
	int s1 = (x1 - x2) * (y3 - y1) + (y1 - y2) * (x1 - x3);
	int t1 = (x1 - x2) * (y4 - y1) + (y1 - y2) * (x1 - x4);
	int s2 = (x3 - x4) * (y1 - y3) + (y3 - y4) * (x3 - x1);
	int t2 = (x3 - x4) * (y2 - y3) + (y3 - y4) * (x3 - x2);
	return s1 * t1 <= 0 && s2 * t2 <= 0;
}

int check_hit_bullet_and_quadrange(Bullet_t b, vector<vector<int>> a) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; j++) {
			int p = (i + 1) % 4;
			int q = (j + 1) % 3;
			vector<vector<int>> s = { b.coordinate[j], b.coordinate[q] };
			vector<vector<int>> t = { a[i], a[p] };
			if (check_cross_line_and_line(s, t)) return i;
		}
	}
	return -1;
}

bool check_ch_in_ground(int x, int y) {
	vector<vector<int>> rolled_ch = format_quadrangle(tank_coordinate_low, x, y);
	return check_quadrangle_in_quadrangle(ground_mate, rolled_ch, ch.coordinate_low);
}

void ch_update() {
	init_ch(ch.x, ch.y, ch.cx, ch.cy, ch.angle_low, ch.angle_up, ch.bullet);
	ch.coordinate_low = roll_quadrangle(ch.coordinate_low, ch.x, ch.y, ch.angle_low);
	ch.coordinate_up = roll_quadrangle(ch.coordinate_up, ch.x, ch.y, ch.angle_up);
	ch.coordinate_turret = roll_quadrangle(ch.coordinate_turret, ch.x, ch.y, ch.angle_up);
	ch.cx = min(ch.coordinate_turret[3][0], ch.coordinate_turret[2][0]) + abs(ch.coordinate_turret[2][0] - ch.coordinate_turret[3][0]) / 2;
	ch.cy = min(ch.coordinate_turret[3][1], ch.coordinate_turret[2][1]) + abs(ch.coordinate_turret[2][1] - ch.coordinate_turret[3][1]) / 2;
	if (!check_ch_in_ground(ch.x, ch.y)) {
		ch.coordinate_low = format_quadrangle(tank_coordinate_low, ch.x, ch.y);
	}
}

void ch_bullet_load_full() {
	for (int i = 0; i < ch_capacity; ++i) {
		ch.bullet.push_back(bullet_generate(0, 0, 0, 0, 0, 0, 0, 0, 0));
	}
}

void ch_bullet_shot(int x, int y) {
	if (!ch.bullet.empty() && flame > ch_last_shot_flame + ch_shot_interval_flame) {
		bullet_flying.push_back(bullet_generate(x, y, x, y, 0, ch.angle_up, 0, 0, 10));
		ch.bullet.pop_front();
		ch_last_shot_flame = flame;
	}
}

void bullet_update() {
	int killed = 0;
	int j = 0;
	for (int i = 0; i < bullet_flying.size(); ++i) {
		if (!check_bullet_in_ground(bullet_flying[j])) {
			++bullet_flying[j].ref;
			if (bullet_flying[j].ref > bullet_ref_limit) {
				bullet_flying.erase(bullet_flying.begin() + j);
				++killed;
				continue;
			}
			Bullet_t bullet = bullet_flying[j];

		}
		j = i - killed;
		Bullet_t bullet = bullet_flying[j];
		float t = radian(bullet_flying[j].angle);
		int sx = bullet.sx;
		int sy = bullet.sy;
		int a = bullet.sx;
		int b = bullet.sy + bullet.movespeed * bullet.md;
		int nx = sx + (a - sx) * cos(t) - (b - sy) * sin(t);
		int ny = sy + (a - sx) * sin(t) + (b - sy) * cos(t);
		++bullet_flying[j].md;
		bullet_flying[j] = bullet_generate(nx, ny, sx, sy, bullet_flying[j].md, bullet.angle, 0, 0, bullet.movespeed);
	}
}

void bullet_draw() {
	for (Bullet_t b : bullet_flying) {
		DrawTriangle(b.coordinate[0][0], b.coordinate[0][1],
			b.coordinate[1][0], b.coordinate[1][1],
			b.coordinate[2][0], b.coordinate[2][1], bw[1], 0);
	}
}

void scope_draw() {
	int length = 6;
	int space = 9;
	float t = radian(ch.angle_up);
	for (int i = 0; i < 100; ++i) {
		int a = ch.cx;
		int b = ch.cy + space + (length + space) * i;
		int b2 = ch.cy + (length + space) * i + space * 2;
		int cx2 = ch.cx + (a - ch.cx) * cos(t) - (b - ch.cy) * sin(t);
		int cy2 = ch.cy + (a - ch.cx) * sin(t) + (b - ch.cy) * cos(t);
		int cx3 = ch.cx + (a - ch.cx) * cos(t) - (b2 - ch.cy) * sin(t);
		int cy3 = ch.cy + (a - ch.cx) * sin(t) + (b2 - ch.cy) * cos(t);
		DrawLine(cx2, cy2, cx3, cy3, pink);
	}
}

void ch_draw() {
	DrawQuadrangle(ch.coordinate_low[0][0], ch.coordinate_low[0][1],
		ch.coordinate_low[1][0], ch.coordinate_low[1][1],
		ch.coordinate_low[2][0], ch.coordinate_low[2][1],
		ch.coordinate_low[3][0], ch.coordinate_low[3][1], red, 0);
	DrawQuadrangle(ch.coordinate_up[0][0], ch.coordinate_up[0][1],
		ch.coordinate_up[1][0], ch.coordinate_up[1][1],
		ch.coordinate_up[2][0], ch.coordinate_up[2][1],
		ch.coordinate_up[3][0], ch.coordinate_up[3][1], red, 0);
	DrawQuadrangle(ch.coordinate_turret[0][0], ch.coordinate_turret[0][1],
		ch.coordinate_turret[1][0], ch.coordinate_turret[1][1],
		ch.coordinate_turret[2][0], ch.coordinate_turret[2][1],
		ch.coordinate_turret[3][0], ch.coordinate_turret[3][1], bw[0], 1
	);
	DrawQuadrangle(ch.coordinate_turret[0][0], ch.coordinate_turret[0][1],
		ch.coordinate_turret[1][0], ch.coordinate_turret[1][1],
		ch.coordinate_turret[2][0], ch.coordinate_turret[2][1],
		ch.coordinate_turret[3][0], ch.coordinate_turret[3][1], red, 0
	);
}

void draw_ground_frame() {
	for (vector<vector<int>> g : ground) {
		DrawQuadrangle(g[0][0], g[0][1],
			g[1][0], g[1][1],
			g[2][0], g[2][1],
			g[3][0], g[3][1], bw[0], 1);
	}
	DrawQuadrangle(ground_mate[0][0], ground_mate[0][1],
		ground_mate[1][0], ground_mate[1][1],
		ground_mate[2][0], ground_mate[2][1],
		ground_mate[3][0], ground_mate[3][1], bw[1], 0);
}

void draw_data() {
}

void update() {
	ch_update();
	bullet_update();
}

void draw() {
	ClearDrawScreen();
	if (ch_scope) scope_draw();
	bullet_draw();
	ch_draw();
	draw_ground_frame();
	draw_data();
	ScreenFlip();
}

void operation() {
	SetMouseDispFlag(FALSE);
	GetHitKeyStateAll(keys);
	if (keys[KEY_INPUT_ESCAPE]) DxLib_End();
	if (keys[KEY_INPUT_F11]) {
		window_mode = !window_mode;
		ChangeWindowMode(window_mode);
		SetMouseDispFlag(FALSE);
	}
	if (keys[KEY_INPUT_F10] && flame%2==0) input_mode = !input_mode;
	DINPUT_JOYSTATE input;
	int MouseX, MouseY;
	GetJoypadDirectInputState(DX_INPUT_PAD1, &input);
	int lx = 0;
	int ly = 0;
	int rx = 0;
	int ry = 0;
	int lt = 0;
	int rt = 0;
	if (input_mode) {
		SetValidMousePointerWindowOutClientAreaMoveFlag(TRUE);
		lx = input.X;
		ly = input.Y;
		rx = input.Rz;
		ry = input.Z;
		lt = input.Buttons[6];
		rt = input.Buttons[7];
	}
	else {
		if (keys[KEY_INPUT_A]) lx = -1000;
		if (keys[KEY_INPUT_D]) lx = 1000;
		if (keys[KEY_INPUT_W]) ly = -1000;
		if (keys[KEY_INPUT_S]) ly = 1000;
		SetValidMousePointerWindowOutClientAreaMoveFlag(FALSE);
		GetMousePoint(&MouseX, &MouseY);
		rx = MouseX - window_w_mid;
		ry = MouseY - window_h_mid;
		if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) lt = 255;
		if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) rt = 255;
	}
	bool ch_move_flag = true;
	if (lx == 0 && ly < 0) {
		ch.angle_low = 180.0f;
	}
	else if (lx == 0 && ly > 0) {
		ch.angle_low = 0.0f;
	}
	else if (lx > 0 && ly == 0) {
		ch.angle_low = 270.0f;
	}
	else if (lx < 0 && ly == 0) {
		ch.angle_low = 90.0f;
	}
	else if (lx != 0 && ly != 0) {
		if (lx <= abs(ly) && lx > 0 && ly < 0) {
			ch.angle_low = degree(atan(ly / lx)) + 270.0f;
		}
		else if (lx > abs(ly) && lx > 0 && ly < 0) {
			ch.angle_low = degree(-atan(lx / ly)) + 180.0f;
		}
		else if (lx >= abs(ly) && lx > 0 && ly > 0) {
			ch.angle_low = degree(-atan(lx / ly)) + 360.0f;
		}
		else if (lx < abs(ly) && lx > 0 && ly > 0) {
			ch.angle_low = degree(atan(ly / lx)) + 270.0f;
		}
		else if (abs(lx) <= ly && lx < 0 && ly > 0) {
			ch.angle_low = degree(atan(ly / lx)) + 90.0f;
		}
		else if (abs(lx) > ly && lx < 0 && ly > 0) {
			ch.angle_low = degree(-atan(lx / ly));
		}
		else if (abs(lx) >= abs(ly) && lx < 0 && ly < 0) {
			ch.angle_low = degree(-atan(lx / ly)) + 180.0f;
		}
		else if (abs(lx) < abs(ly) && lx < 0 && ly < 0) {
			ch.angle_low = degree(atan(ly / lx)) + 90.0f;
		}
	}
	else if (lx == 0 && ly == 0) {
		ch_move_flag = false;
	}
	if (rx == 0 && ry < 0) {
		ch.angle_up = 180.0f;
	}
	else if (rx == 0 && ry > 0) {
		ch.angle_up = 0.0f;
	}
	else if (rx > 0 && ry == 0) {
		ch.angle_up = 270.0f;
	}
	else if (rx < 0 && ry == 0) {
		ch.angle_up = 90.0f;
	}
	else if (rx != 0 && ry != 0) {
		if (rx <= abs(ry) && rx > 0 && ry < 0) {
			ch.angle_up = degree(atan(ry / rx)) + 270.0f;
		}
		else if (rx > abs(ry) && rx > 0 && ry < 0) {
			ch.angle_up = degree(-atan(rx / ry)) + 180.0f;
		}
		else if (rx >= abs(ry) && rx > 0 && ry > 0) {
			ch.angle_up = degree(-atan(rx / ry)) + 360.0f;
		}
		else if (rx < abs(ry) && rx > 0 && ry > 0) {
			ch.angle_up = degree(atan(ry / rx)) + 270.0f;
		}
		else if (abs(rx) <= ry && rx < 0 && ry > 0) {
			ch.angle_up = degree(atan(ry / rx)) + 90.0f;
		}
		else if (abs(rx) > ry && rx < 0 && ry > 0) {
			ch.angle_up = degree(-atan(rx / ry));
		}
		else if (abs(rx) >= abs(ry) && rx < 0 && ry < 0) {
			ch.angle_up = degree(-atan(rx / ry)) + 180.0f;
		}
		else if (abs(rx) < abs(ry) && rx < 0 && ry < 0) {
			ch.angle_up = degree(atan(ry / rx)) + 90.0f;
		}
	}
	int predicted_x = ch.x;
	int predicted_y = ch.y;
	if (ch_move_flag) {
		float t = radian(ch.angle_low);
		int a = ch.x;
		int b = ch.y + ch.move_speed;
		predicted_x = ch.x + (a - ch.x) * cos(t) - (b - ch.y) * sin(t);
		predicted_y = ch.y + (a - ch.x) * sin(t) + (b - ch.y) * cos(t);
	}
	init_ch(ch.x, ch.y, ch.cx, ch.cy, ch.angle_low, ch.angle_up, ch.bullet);
	if (check_ch_in_ground(predicted_x, predicted_y)) {
		ch.x = predicted_x;
		ch.y = predicted_y;
	}
	if (lt > 0) {
		ch_scope = true;
	}
	else {
		ch_scope = false;
	}
	if (rt > 0) {
		if (!ch.bullet.empty()) {
			ch_bullet_shot(ch.cx, ch.cy);
		}
	}
}

void first(int x, int y) {
	deque<Bullet_t> deq = {};
	init_ch(x, y, 0, 0, 270.0f, 270.0f, deq);
	for (vector<vector<int>> g : ground) {
		quadrange_all.push_back(g);
	}
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	ChangeWindowMode(window_mode);
	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);
	SetWindowText("パッド推奨");
	if (DxLib_Init() == -1) return -1;
	first(320, 240);
	ch_bullet_load_full();
	while (true) {
		if (ProcessMessage() == -1) return -1;
		operation();
		update();
		draw();
		++flame;
	}
	DxLib_End();
	return 0;
}