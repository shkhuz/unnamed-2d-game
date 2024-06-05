int wallcon[] = {
    0b000'010'010'000,
    0b000'000'010'010,
    0b010'000'010'000,
    0b000'010'010'010,
    0b010'010'000'010,
    0b000'000'000'010,
    0b010'000'000'000,
    0b010'010'010'010,

    0b010'010'000'000,
    0b010'000'000'010,
    0b000'010'000'010,
    0b010'010'010'000,
    0b010'000'010'010,
    0b000'000'010'000,
    0b000'010'000'000,
    0b000'011'110'000,

    0b111'110'011'111,
    0b111'111'110'011,
    0b111'110'010'011,
    0b010'011'111'110,
    0b011'111'110'000,
    0b110'000'011'111,
    0b111'111'111'111,
    0b011'110'000'000,

    0b110'011'111'111,
    0b011'111'111'110,
    0b110'010'011'111,
    0b011'111'110'010,
    0b000'011'111'110,
    0b111'110'000'011,
    0b000'000'000'000,
    0b000'000'011'110,

    0b010'011'110'010,
    0b010'010'011'110,
    0b010'011'110'000,
    0b010'000'011'110,
    0b000'011'110'010,
    0b000'010'011'110,
    0b011'110'011'110,
    0b110'000'000'011,

    0b011'110'010'010,
    0b110'010'010'011,
    0b011'110'010'000,
    0b110'000'010'011,
    0b011'110'000'010,
    0b110'010'000'011,
    0b110'011'110'011,
    -1,
};

enum WallconDir {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

u8 wallcon_get_up(int tile)    { return (tile>>9) & 0b111; }
u8 wallcon_get_right(int tile) { return (tile>>6) & 0b111; }
u8 wallcon_get_down(int tile)  { return (tile>>3) & 0b111; }
u8 wallcon_get_left(int tile)  { return (tile>>0) & 0b111; }

void wallcon_set_up(int* tile, u8 c)    { *tile = (*tile & (~(0b111<<9))) | ((int)c<<9); }
void wallcon_set_right(int* tile, u8 c) { *tile = (*tile & (~(0b111<<6))) | ((int)c<<6); }
void wallcon_set_down(int* tile, u8 c)  { *tile = (*tile & (~(0b111<<3))) | ((int)c<<3); }
void wallcon_set_left(int* tile, u8 c)  { *tile = (*tile & (~(0b111<<0))) | ((int)c<<0); }

int wallcon_get_wall_with_neis(int utile, int rtile, int dtile, int ltile) {
    int wall = 0;
    wallcon_set_up(&wall,    wallcon_get_down(utile));
    wallcon_set_right(&wall, wallcon_get_left(rtile));
    wallcon_set_down(&wall,  wallcon_get_up(dtile));
    wallcon_set_left(&wall,  wallcon_get_right(ltile));
    return wall;
}

void join_walls(int* current, int* nei, WallconDir rel) {
    switch (rel) {
        case UP:
            wallcon_set_up(current, 0b010);
            wallcon_set_down(nei, 0b010);
            break;
        case RIGHT:
            wallcon_set_right(current, 0b010);
            wallcon_set_left(nei, 0b010);
            break;
        case DOWN:
            wallcon_set_down(current, 0b010);
            wallcon_set_up(nei, 0b010);
            break;
        case LEFT:
            wallcon_set_left(current, 0b010);
            wallcon_set_right(nei, 0b010);
            break;
    }
}

void disjoin_walls(int* current, int* nei, WallconDir rel) {
    switch (rel) {
        case UP:
            if (current) wallcon_set_up(current, 0b000);
            wallcon_set_down(nei, 0b000);
            break;
        case RIGHT:
            if (current) wallcon_set_right(current, 0b000);
            wallcon_set_left(nei, 0b000);
            break;
        case DOWN:
            if (current) wallcon_set_down(current, 0b000);
            wallcon_set_up(nei, 0b000);
            break;
        case LEFT:
            if (current) wallcon_set_left(current, 0b000);
            wallcon_set_right(nei, 0b000);
            break;
    }
}

std::unordered_map<int, glm::ivec2> wallcon_atlas_map;

void init_wallcon() {
    glm::ivec2 wall_atlas_pos = glm::vec2(4, 2);
    glm::ivec2 wall_atlas_size = glm::vec2(8, 6);
    for (int y = 0; y < wall_atlas_size.y; y++) {
        for (int x = 0; x < wall_atlas_size.x; x++) {
            int id = wallcon[y*wall_atlas_size.x+x];
            wallcon_atlas_map[id] =
                glm::ivec2(wall_atlas_pos.x+x, wall_atlas_pos.y+y);
        }
    }
}

glm::ivec2 wallcon_connect_to_atlas(int con) {
    return wallcon_atlas_map[con];
}

int wallcon_atlas_to_connect(glm::ivec2 atpos) {
    auto it = std::find_if(std::begin(wallcon_atlas_map), std::end(wallcon_atlas_map),
        [&atpos](auto&& p) { return p.second == atpos; });
    if (it != std::end(wallcon_atlas_map)) return it->first;
    return -1;
}
