float modelindex_playernail;
float modelindex_playersnail;
float modelindex_playershot;
float modelindex_playersshot;
float modelindex_playerneedler;
float modelindex_playerrocket;
float modelindex_playersniper;
float modelindex_playerplasma;
float modelindex_playerbr;
float modelindex_playersword;
float modelindex_dead;

void () CheckModel
{
    if (self.health < 0)
          self.modelindex = modelindex_dead;
    else if (self.weapon == WEAPON_PISTOL)
         self.modelindex = modelindex_playershot;
    else if (self.weapon == WEAPON_AR)
         self.modelindex = modelindex_playernail;
    else if (self.weapon == WEAPON_UZI)
         self.modelindex = modelindex_playersnail;
    else if (self.weapon == WEAPON_SHOT)
         self.modelindex = modelindex_playersshot;
    else if (self.weapon == WEAPON_NEEDLER)
         self.modelindex = modelindex_playerneedler;
    else if (self.weapon == WEAPON_SNIPER)
        self.modelindex = modelindex_playersniper;
    else if (self.weapon == WEAPON_RL)
        self.modelindex = modelindex_playerrocket;
    else if (self.weapon == WEAPON_PPISTOL)
        self.modelindex = modelindex_playerplasma;
    else if (self.weapon == WEAPON_PRIFLE)
        self.modelindex = modelindex_playerplasma; //todo: replace with model for rifle
    else if (self.weapon == WEAPON_BR)
        self.modelindex = modelindex_playerbr;
    else if (self.weapon == WEAPON_SWORD)
        self.modelindex = modelindex_playersword; //todo: replace with model for sword
};