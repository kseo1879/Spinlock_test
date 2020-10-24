struct tas {
    
};

void tas_init(struct tat *t);

int tas_lock(struct tas *t);

int tas_unlock(struct tas *t);

int tas_destory(struct tas *t);
