/*
===========================================================================
MovieDuels: which animation style a model plays (Update 9, 2026-11-22)
===========================================================================

Every model.glm names the humanoid animation set it was made for (e.g. models/players/_humanoid_ani/_humanoid).
All those sets' animations are in the Master _humanoid set as NAME_SET (BOTH_RUN2_ANI...), and each animation style
plays its set's versions. So a model's style follows from the set its model.glm names: a model made for
_humanoid_ani plays CS_ANAKIN, whatever its NPC file says, and it keeps doing so when the per-character set folders
are gone (the name in model.glm stays). Models on the plain _humanoid set (or anything else) get 0 here and keep the
style their NPC file / model table gives them.

C and C++ (SP game, MP game / cgame). Style numbers: SP Animationstyles_t (teams.h) = MP animStyle_t (bg_public.h).
*/

#ifndef MD_ANIMSTYLE_SETS_H
#define MD_ANIMSTYLE_SETS_H

typedef struct mdAnimSetStyle_s
{
	const char* folder; // models/players/<folder>/
	int style;          // CS_* / ANIMSTYLE_*
} mdAnimSetStyle_t;

static const mdAnimSetStyle_t md_animSetStyles[] =
{
	{ "_humanoid_ani", 1 },      // CS_ANAKIN
	{ "_humanoid_bdroid", 2 },   // CS_BATTLEDROID
	{ "_humanoid_ben", 3 },      // CS_BENKENOBI
	{ "_humanoid_cal", 4 },      // CS_CAL_KESTIS
	{ "_humanoid_clo", 5 },      // CS_CLONETROOPER
	{ "_humanoid_df2", 6 },      // CS_DARKFORCES2
	{ "_humanoid_dooku", 7 },    // CS_COUNT_DOOKU
	{ "_humanoid_galen", 8 },    // CS_GALEN_MAREK
	{ "_humanoid_gon", 9 },      // CS_QUI_GON_JINN
	{ "_humanoid_grievous", 10 },// CS_GRIEVOUS
	{ "_humanoid_jango", 11 },   // CS_JANGO
	{ "_humanoid_kotor", 12 },   // CS_KOTOR
	{ "_humanoid_luke", 13 },    // CS_LUKE_SKYWALKER
	{ "_humanoid_mace", 14 },    // CS_MACE_WINDU
	{ "_humanoid_maul", 15 },    // CS_MAUL
	{ "_humanoid_md", 16 },      // CS_MOVIEDUELS
	{ "_humanoid_obi", 17 },     // CS_OBIWAN
	{ "_humanoid_obi3", 18 },    // CS_OBIWAN_EP3
	{ "_humanoid_pal", 19 },     // CS_PALPATINE
	{ "_humanoid_reb", 20 },     // CS_REBELS
	{ "_humanoid_ren", 21 },     // CS_KYLO_REN
	{ "_humanoid_rey", 22 },     // CS_REY
	{ "_humanoid_vader", 23 },   // CS_VADER
	{ "_humanoid_yoda", 24 },    // CS_YODA
};

// The style of the set in an animation set path (model.glm animName, "models/players/_humanoid_ani/_humanoid"):
// 0 when it is no character set. Whole folder names only ("_humanoid_obi" is not "_humanoid_obi3").
static inline int MD_StyleForAnimSet(const char* gla_name)
{
	const char* folder;
	const char* end;
	int len;
	int i;

	if (!gla_name || !gla_name[0])
	{
		return 0;
	}
	folder = gla_name;
	if (!Q_stricmpn(folder, "models/players/", 15))
	{
		folder += 15;
	}
	end = folder;
	while (*end && *end != '/' && *end != '\\')
	{
		end++;
	}
	len = (int)(end - folder);
	for (i = 0; i < (int)(sizeof md_animSetStyles / sizeof md_animSetStyles[0]); i++)
	{
		if ((int)strlen(md_animSetStyles[i].folder) == len && !Q_stricmpn(folder, md_animSetStyles[i].folder, len))
		{
			return md_animSetStyles[i].style;
		}
	}
	return 0;
}

// The animation set a model.glm names: its header holds animName[64] at byte 72 (ident, version, name[64]).
// buf = the first 136 (or more) bytes of the file, len = how many were read.
static inline void MD_GlmAnimName(const char* buf, const int len, char* out, const int out_size)
{
	int i;

	out[0] = 0;
	if (len < 136 || buf[0] != '2' || buf[1] != 'L' || buf[2] != 'G' || buf[3] != 'M')
	{
		return;
	}
	for (i = 0; i < 64 && i < out_size - 1 && buf[72 + i]; i++)
	{
		out[i] = buf[72 + i];
	}
	out[i] = 0;
}

#endif // MD_ANIMSTYLE_SETS_H
