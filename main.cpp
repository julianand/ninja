#include <allegro.h>

#define X 25
#define Y 15

#define PIX 40

BITMAP *buffer;
BITMAP *fondo;
BITMAP *roca;
BITMAP *star;

BITMAP *pj_bitmap;

struct Pj {
	int tamx;
	int tamy;
	
	int x;
	int y;
	
	int alt;
	int dir;
	
	bool salto;
	int dsalto;
	bool air;
	int dair;
	
	int vidas;
};

struct Obstaculo {
	int x;
	int y;
	int vel;
	int alt;
	int tipo; //0: obstaculo	1: premio	2: nada
	bool activo;
	
	struct Obstaculo *sig;
};

struct Obstaculo *obs_ptr;
struct Obstaculo *obs_aux;
struct Pj *pj;

int interval = 57;
int act = 0;
int espera = 100;
int nivel = 0;
int puntaje = 0;

char mapa[Y][X+1] = {
	"f                        ",
	"                         ",
	"                         ",
	"                         ",
	"                         ",
	"                         ",
	"            xxxxx        ",
	"                         ",
	"                         ",
	"                         ",
	"    xxxxxxxx     xxxxx   ",
	"                         ",
	"                         ",
	"                         ",
	"xxxxxxxxxxxxxxxxxxxxxxxxx"
};

void dibujar() {
	for(int y = 0; y < Y; y++) {
		for(int x = 0; x < X; x++) {
			if(mapa[y][x] == 'f') draw_sprite(buffer,fondo,x*PIX,y*PIX);
			if(mapa[y][x] == 'x') draw_sprite(buffer,roca,x*PIX,y*PIX);
		}
	}
	
	BITMAP *btmp = create_bitmap(pj->tamx,pj->tamy);
	blit(pj_bitmap,btmp,pj->alt*pj->tamx,pj->dir*pj->tamy,0,0,pj->tamx,pj->tamy);
	draw_sprite(buffer,btmp,pj->x,pj->y);
	
	struct Obstaculo *aux = obs_ptr;
	if(aux != NULL) {
		do {
			if(aux->activo) {
				BITMAP *btmp = create_bitmap(45,45);
				blit(star, btmp, aux->alt * 45 , aux->tipo * 45, 0, 0, 45, 45);
				draw_sprite(buffer,btmp,aux->x,aux->y);
			}
			aux = aux->sig;
		} while(aux != obs_ptr);
	}
	
	textprintf_ex(buffer, font, 10, 10, palette_color[15], -1, "Vidas: %d", pj->vidas);
	textprintf_ex(buffer, font, 100, 10, palette_color[15], -1, "Nivel: %d", nivel);
	textprintf_ex(buffer, font, 200, 10, palette_color[15], -1, "Puntaje: %d", puntaje);
	
	textprintf_ex(buffer, font, 10, 20, palette_color[15], -1, "%d", act);
	textprintf_ex(buffer, font, 100, 20, palette_color[15], -1, "Intervalo: %d", interval);
	
	draw_sprite(screen,buffer,0,0);
}

bool verificaru(int x, int y, int tamx, int tamy, char obj) {
	bool b = true;
	for(int i = x; i < x+tamx; i++) {
		for(int j = y; j < y+tamy; j++) {
			if(mapa[j/PIX][i/PIX] != obj) b = false;
		}
	}
	return b;
}

bool verificar_o(int x, int y, int tamx, int tamy, int xo, int yo, int tamxo, int tamyo) {
	bool b = false;
	for(int i = x; i < x+tamx; i++) {
		for(int j = y; j < y+tamy; j++) {
			if((i > xo && i < xo+tamxo) && (j > yo && j < yo+tamyo)) b = true;
		}
	}
	return b;
}

void aire(int tamx, int tamy) {
	if(pj->salto) {
		if(verificaru(pj->x, pj->y - pj->dsalto, tamx, tamy, ' ')) pj->y -= pj->dsalto;
		else {
			while(!verificaru(pj->x, pj->y - pj->dsalto, tamx, tamy, ' ')) pj->dsalto--;
			pj->y -= pj->dsalto;
		}
		pj->dsalto--;
		if(pj->dsalto <= 0) pj->salto = false;
	}
	//caida
	if(verificaru(pj->x, pj->y + 1, tamx, tamy, ' ') && !pj->salto && !pj->air) {
		pj->air = true;
		pj->dair = 1;
	}
	if(pj->air) {
		if(verificaru(pj->x, pj->y + pj->dair, tamx, tamy, ' ')) pj->y += pj->dair;
		else {
			while(!verificaru(pj->x, pj->y + pj->dair, tamx, tamy, ' ')) pj->dair--;
			pj->y += pj->dair;
			pj->air = false;
			pj->dir-=4;
		}
		pj->dair++;
	}
}

int mov = 8;
void mover() {
	if((!pj->salto && !pj->air) && key[KEY_RIGHT]) {
		mov = 8;
		pj->dir = 0;
	}
	if((!pj->salto && !pj->air) && key[KEY_LEFT]) {
		mov = -8;
		pj->dir = 1;
	}
	
	pj->alt = 1 - pj->alt;
	if(verificaru(pj->x+mov,pj->y,pj->tamx,pj->tamy,' ')) {
		if(pj->x + mov > 0) pj->x+=mov;
	}
	
	if(key[KEY_UP] && !pj->salto && !pj->air) {
		pj->salto = true;
		pj->dsalto = 20;
	}
	
	if(pj->salto && pj->dir < 2) pj->dir+=2;
	
	if(pj->air && pj->dir < 2) pj->dir+=4;
	else if(pj->air && pj->dir < 4) pj->dir+=2;
	
	
	aire(pj->tamx,pj->tamy);
}

struct Obstaculo *crear_obstaculo(int y, int tipo) {
	struct Obstaculo *o = (struct Obstaculo *) malloc(sizeof(struct Obstaculo));
	o->x = 24*PIX;
	o->y = y;
	o->tipo = tipo;
	o->activo = false;
	o->alt = 0;
	o->vel = 9+nivel;
	
	return o;
}

void pos_obs(int v[], int limit, int tipo) {
	srand(time(NULL));
	for(int i = 0; i < limit;) {
		int a = rand()%50;
		if(v[a] == 2) {
			v[a] = tipo;
			i++;
		}
	}
}

void iniciarLista() {
	srand(time(NULL));
	struct Obstaculo *aux;
	int pos[50];
	
	for(int i = 0; i < 50; i++) {
		pos[i] = 2;
	}
	
	pos_obs(pos, 15, 0);
	pos_obs(pos, 15, 1);
	
	for(int i = 0; i < 50; i++) {
		int rnd = rand()%3;
		
		int y = PIX;
		if(rnd == 0) y *= 4;
		if(rnd == 1) y *= 8;
		if(rnd == 2) y *= 12;
		
		if(i == 0) {
			aux = crear_obstaculo(y, pos[i]);
			aux->sig = aux;
			obs_ptr = aux;
		}
		else {
			aux->sig = crear_obstaculo(y, pos[i]);
			aux = aux->sig;
			aux->sig = obs_ptr;
		}
	}
}

int c_interval;
void generar_obstaculo() {
	//inicio de nivel
	if(obs_aux == obs_ptr && espera == 100) {
		nivel++;
		iniciarLista();
		obs_aux = obs_ptr;
		interval-=7;
		act = 0;
	}
	
	if(c_interval >= interval && espera == 100) {
		obs_aux->activo = true;
		obs_aux = obs_aux->sig;
		if(obs_aux == obs_ptr) espera = 0;
		c_interval = 0;
		act++;
	}
	
	espera++;
	if(espera > 100) espera = 100;
	
	c_interval++;
}

void mover_obstaculo() {
	struct Obstaculo *aux = obs_ptr;
	do {
		if(aux->activo) {
			aux->alt = 1 - aux->alt;
			aux->x -= aux->vel;
		}
		if(aux->x <= -45) aux->activo = false;
		aux = aux->sig;
	} while(aux != obs_ptr);
}

void choque() {
	struct Obstaculo *aux = obs_ptr;
	do {
		if(aux->activo && verificar_o(pj->x, pj->y, pj->tamx, pj->tamy, aux->x, aux->y, 45, 45)) {
			aux->activo = false;
			if(aux->tipo == 0) pj->vidas--;
			if(aux->tipo == 1) puntaje+=100;
		}
		aux = aux->sig;
	} while(aux != obs_ptr);
}

void init();
void deinit();

int main() {
	init();
	
	//musica = load_sample("musica.wav");
	
	obs_ptr = NULL;
	obs_aux = obs_ptr;
	c_interval = interval;
	
	pj = (struct Pj *) malloc(sizeof(struct Pj));
	pj->air = false;
	pj->salto = false;
	pj->alt = 0;
	pj->dir = 0;
	pj->dair = 0;
	pj->dsalto = 0;
	pj->tamx = 50;
	pj->tamy = 52;
	pj->x = PIX;
	pj->y = 13*PIX-12;
	pj->vidas = 3;
	
	buffer = create_bitmap(X*PIX,Y*PIX);
	fondo = load_bitmap("fondo.bmp", NULL);
	roca = load_bitmap("roca.bmp", NULL);
	star = load_bitmap("star.bmp", NULL);
	
	pj_bitmap = load_bitmap("pj.bmp", NULL);

	//play_sample(musica, 255, 127, 1000, 1);
	while (!key[KEY_ESC] && pj->vidas > 0) {
		mover();
		generar_obstaculo();
		mover_obstaculo();
		choque();
		
		dibujar();
		clear(buffer);
		rest(20);
	}
	
	while(!key[KEY_ESC]) {
		textprintf_ex(screen, font, 400, 10, palette_color[14], -1, "Juego terminado! Puntaje total: %d", puntaje);
	}

	deinit();
	return 0;
}
END_OF_MAIN()

void init() {
	int depth, res;
	allegro_init();
	depth = desktop_color_depth();
	if (depth == 0) depth = 32;
	set_color_depth(depth);
	res = set_gfx_mode(GFX_AUTODETECT_WINDOWED, X*PIX, Y*PIX, 0, 0);
	if (res != 0) {
		allegro_message(allegro_error);
		exit(-1);
	}
	
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0) {
       allegro_message("Error: inicializando sistema de sonido\n%s\n", allegro_error);
       exit(-1);
    }
	
	install_keyboard();
	/* add other initializations here */
}

void deinit() {
	clear_keybuf();
	/* add other deinitializations here */
}
