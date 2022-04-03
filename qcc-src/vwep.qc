float modelindex_playernail;
float modelindex_playersnail;
float modelindex_playershot;
float modelindex_playersshot;
float modelindex_playerneedler;
float modelindex_playerrocket;
float modelindex_playersniper;
float modelindex_playerplasma;

void () CheckModel
{
   if (self.weapon == IT_SHOTGUN)
      self.modelindex = modelindex_playershot;
   else if (self.weapon == IT_NAILGUN)
      self.modelindex = modelindex_playernail;
   else if (self.weapon == IT_SUPER_NAILGUN)
      self.modelindex = modelindex_playersnail;
   else if (self.weapon == IT_SUPER_SHOTGUN)
      self.modelindex = modelindex_playersshot;
   else if (self.weapon == WEAPON_NEEDLER)
      self.modelindex = modelindex_playerneedler;
   else if (self.weapon == WEAPON_SNIPER)
      self.modelindex = modelindex_playersniper;
   else if (self.weapon == WEAPON_RL)
      self.modelindex = modelindex_playerrocket;
   else if (self.weapon == WEAPON_PPISTOL)
      self.modelindex = modelindex_playerplasma;
};