/*	MikMod sound library
	(c) 1998, 1999, 2000, 2001, 2002 Miodrag Vallat and others - see file
	AUTHORS for complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id$

  Screamtracker 2 (STM) module loader

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>

#include "mikmod_internals.h"

#ifdef SUNOS
extern int fprintf(FILE *, const char *, ...);
#endif

/*========== Module structure */

/* sample information */
typedef struct STMSAMPLE {
	CHAR  filename[12];
	UBYTE unused;       /* 0x00 */
	UBYTE instdisk;     /* Instrument disk */
	UWORD reserved;
	UWORD length;       /* Sample length */
	UWORD loopbeg;      /* Loop start point */
	UWORD loopend;      /* Loop end point */
	UBYTE volume;       /* Volume */
	UBYTE reserved2;
	UWORD c2spd;        /* Good old c2spd */
	ULONG reserved3;
	UWORD isa;
} STMSAMPLE;

/* header */
typedef struct STMHEADER {
	CHAR  songname[20];
	CHAR  trackername[8]; /* !Scream! for ST 2.xx  */
	UBYTE unused;         /* 0x1A  */
	UBYTE filetype;       /* 1=song, 2=module */
	UBYTE ver_major;
	UBYTE ver_minor;
	UBYTE inittempo;      /* initspeed= stm inittempo>>4  */
	UBYTE numpat;         /* number of patterns  */
	UBYTE globalvol;
	UBYTE reserved[13];
	STMSAMPLE sample[31]; /* STM sample data */
	UBYTE patorder[128];  /* Docs say 64 - actually 128 */
} STMHEADER;

typedef struct STMNOTE {
	UBYTE note,insvol,volcmd,cmdinf;
} STMNOTE;

/*========== Loader variables */

static STMNOTE *stmbuf = NULL;
static STMHEADER *mh = NULL;

/* tracker identifiers */
static const CHAR * STM_Version[STM_NTRACKERS] = {
	"Screamtracker 2",
	"Converted by MOD2STM (STM format)",
	"Wuzamod (STM format)"
};

/*========== Loader code */

static int STM_Test(void)
{
	UBYTE str[44];
	int t;

	memset(str,0,44);
	_mm_fseek(modreader,20,SEEK_SET);
	_mm_read_UBYTES(str,44,modreader);
	if(str[9]!=2) return 0;	/* STM Module = filetype 2 */

	/* Prevent false positives for S3M files */
	if(!memcmp(str+40,"SCRM",4))
		return 0;

	for (t=0;t<STM_NTRACKERS;t++)
		if(!memcmp(str,STM_Signatures[t],8))
			return 1;

	return 0;
}

static int STM_Init(void)
{
	if(!(mh=(STMHEADER*)MikMod_malloc(sizeof(STMHEADER)))) return 0;
	if(!(stmbuf=(STMNOTE*)MikMod_calloc(64U*4,sizeof(STMNOTE)))) return 0;

	return 1;
}

static void STM_Cleanup(void)
{
	MikMod_free(mh);
	MikMod_free(stmbuf);
	mh=NULL;
	stmbuf=NULL;
}

static void STM_ConvertNote(STMNOTE *n)
{
	UBYTE note,ins,vol,cmd,inf;

	/* extract the various information from the 4 bytes that make up a note */
	note = n->note;
	ins  = n->insvol>>3;
	vol  = (n->insvol&7)+((n->volcmd&0x70)>>1);
	cmd  = n->volcmd&15;
	inf  = n->cmdinf;

	if((ins)&&(ins<32)) UniInstrument(ins-1);

	/* special values of [SBYTE0] are handled here
	   we have no idea if these strange values will ever be encountered.
	   but it appears as those stms sound correct. */
	if((note==254)||(note==252)) {
		UniPTEffect(0xc,0); /* note cut */
		n->volcmd|=0x80;
	} else
		/* if note < 251, then all three bytes are stored in the file */
	  if(note<251) UniNote((((note>>4)+2)*OCTAVE)+(note&0xf));

	if((!(n->volcmd&0x80))&&(vol<65)) UniPTEffect(0xc,vol);
	if(cmd!=255)
		switch(cmd) {
			case 1:	/* Axx set speed to xx */
				UniPTEffect(0xf,inf>>4);
				break;
			case 2:	/* Bxx position jump */
				UniPTEffect(0xb,inf);
				break;
			case 3:	/* Cxx patternbreak to row xx */
				UniPTEffect(0xd,(((inf&0xf0)>>4)*10)+(inf&0xf));
				break;
			case 4:	/* Dxy volumeslide */
				UniEffect(UNI_S3MEFFECTD,inf);
				break;
			case 5:	/* Exy toneslide down */
				UniEffect(UNI_S3MEFFECTE,inf);
				break;
			case 6:	/* Fxy toneslide up */
				UniEffect(UNI_S3MEFFECTF,inf);
				break;
			case 7:	/* Gxx Tone portamento,speed xx */
				UniPTEffect(0x3,inf);
				break;
			case 8:	/* Hxy vibrato */
				UniPTEffect(0x4,inf);
				break;
			case 9:	/* Ixy tremor, ontime x, offtime y */
				UniEffect(UNI_S3MEFFECTI,inf);
			break;
			case 0:		/* protracker arpeggio */
				if(!inf) break;
				/* fall through */
			case 0xa:	/* Jxy arpeggio */
				UniPTEffect(0x0,inf);
				break;
			case 0xb:	/* Kxy Dual command H00 & Dxy */
				UniPTEffect(0x4,0);
				UniEffect(UNI_S3MEFFECTD,inf);
				break;
			case 0xc:	/* Lxy Dual command G00 & Dxy */
				UniPTEffect(0x3,0);
				UniEffect(UNI_S3MEFFECTD,inf);
				break;
			/* Support all these above, since ST2 can LOAD these values but can
			   actually only play up to J - and J is only half-way implemented
			   in ST2 */
			case 0x18:	/* Xxx amiga panning command 8xx */
				UniPTEffect(0x8,inf);
				of.flags |= UF_PANNING;
				break;
		}
}

static UBYTE *STM_ConvertTrack(STMNOTE *n)
{
	int t;

	UniReset();
	for(t=0;t<64;t++) {
		STM_ConvertNote(n);
		UniNewline();
		n+=of.numchn;
	}
	return UniDup();
}

static int STM_LoadPatterns(unsigned int pattoload)
{
	unsigned int t,s,tracks=0;

	if(!AllocPatterns()) return 0;
	if(!AllocTracks()) return 0;

	/* Allocate temporary buffer for loading and converting the patterns */
	for(t=0;t<pattoload;t++) {
		for(s=0;s<(64U*of.numchn);s++) {
			stmbuf[s].note   = _mm_read_UBYTE(modreader);
			stmbuf[s].insvol = _mm_read_UBYTE(modreader);
			stmbuf[s].volcmd = _mm_read_UBYTE(modreader);
			stmbuf[s].cmdinf = _mm_read_UBYTE(modreader);
		}

		if(_mm_eof(modreader)) {
			_mm_errno = MMERR_LOADING_PATTERN;
			return 0;
		}

		for(s=0;s<of.numchn;s++)
			if(!(of.tracks[tracks++]=STM_ConvertTrack(stmbuf+s))) return 0;
	}
	return 1;
}

static int STM_Load(int curious)
{
	int blankpattern=0;
	int pattoload;
	int t;
	ULONG samplestart;
	ULONG sampleend;
	SAMPLE *q;
	(void)curious;
	/* try to read stm header */
	_mm_read_string(mh->songname,20,modreader);
	_mm_read_string(mh->trackername,8,modreader);
	mh->unused      =_mm_read_UBYTE(modreader);
	mh->filetype    =_mm_read_UBYTE(modreader);
	mh->ver_major   =_mm_read_UBYTE(modreader);
	mh->ver_minor   =_mm_read_UBYTE(modreader);
	mh->inittempo   =_mm_read_UBYTE(modreader);
	if(!mh->inittempo) {
		_mm_errno=MMERR_NOT_A_MODULE;
		return 0;
	}
	mh->numpat      =_mm_read_UBYTE(modreader);
	mh->globalvol   =_mm_read_UBYTE(modreader);
	_mm_read_UBYTES(mh->reserved,13,modreader);
	if(mh->numpat > 128) {
		_mm_errno = MMERR_NOT_A_MODULE;
		return 0;
	}

	for(t=0;t<31;t++) {
		STMSAMPLE *s=&mh->sample[t];	/* STM sample data */

		_mm_read_string(s->filename,12,modreader);
		s->unused   =_mm_read_UBYTE(modreader);
		s->instdisk =_mm_read_UBYTE(modreader);
		s->reserved =_mm_read_I_UWORD(modreader);
		s->length   =_mm_read_I_UWORD(modreader);
		s->loopbeg  =_mm_read_I_UWORD(modreader);
		s->loopend  =_mm_read_I_UWORD(modreader);
		s->volume   =_mm_read_UBYTE(modreader);
		s->reserved2=_mm_read_UBYTE(modreader);
		s->c2spd    =_mm_read_I_UWORD(modreader);
		s->reserved3=_mm_read_I_ULONG(modreader);
		s->isa      =_mm_read_I_UWORD(modreader);
	}
	_mm_read_UBYTES(mh->patorder,128,modreader);

	if(_mm_eof(modreader)) {
		_mm_errno = MMERR_LOADING_HEADER;
		return 0;
	}

	/* set module variables */
	for(t=0;t<STM_NTRACKERS;t++)
		if(!memcmp(mh->trackername,STM_Signatures[t],8)) break;
	of.modtype   = MikMod_strdup(STM_Version[t]);
	of.songname  = DupStr(mh->songname,20,1); /* make a cstr of songname */
	of.numpat    = mh->numpat;
	of.inittempo = 125;                     /* mh->inittempo+0x1c; */
	of.initspeed = mh->inittempo>>4;
	of.numchn    = 4;                       /* get number of channels */
	of.reppos    = 0;
	of.flags    |= UF_S3MSLIDES;
	of.bpmlimit  = 32;

	t=0;
	if(!AllocPositions(0x80)) return 0;
	/* 99 terminates the patorder list */
	while(mh->patorder[t]<99) {
		of.positions[t]=mh->patorder[t];

		/* Screamtracker 2 treaks patterns >= numpat as blank patterns.
		 * Example modules: jimmy.stm, Rauno/dogs.stm, Skaven/hevijanis istu maas.stm.
		 *
		 * Patterns>=64 have unpredictable behavior in Screamtracker 2,
		 * but nothing seems to rely on them, so they're OK to blank too.
		 */
		if(of.positions[t]>=mh->numpat) {
			of.positions[t]=mh->numpat;
			blankpattern=1;
		}

		if(++t == 0x80) {
			_mm_errno = MMERR_NOT_A_MODULE;
			return 0;
		}
	}
	/* Allocate an extra blank pattern if the module references one. */
	pattoload=of.numpat;
	if(blankpattern) of.numpat++;
	of.numpos=t;
	of.numtrk=of.numpat*of.numchn;
	of.numins=of.numsmp=31;

	if(!AllocSamples()) return 0;
	if(!STM_LoadPatterns(pattoload)) return 0;

	samplestart=_mm_ftell(modreader);
	_mm_fseek(modreader,0,SEEK_END);
	sampleend=_mm_ftell(modreader);

	for(q=of.samples,t=0;t<of.numsmp;t++,q++) {
		/* load sample info */
		q->samplename = DupStr(mh->sample[t].filename,12,1);
		q->speed      = (mh->sample[t].c2spd * 8363) / 8448;
		q->volume     = mh->sample[t].volume;
		q->length     = mh->sample[t].length;
		if (!mh->sample[t].volume || q->length==1) q->length=0;
		q->loopstart  = mh->sample[t].loopbeg;
		q->loopend    = mh->sample[t].loopend;
		q->seekpos    = mh->sample[t].reserved << 4;

		/* Sanity checks to make sure samples are bounded within the file. */
		if(q->length) {
			if(q->seekpos<samplestart) {
#ifdef MIKMOD_DEBUG
				fprintf(stderr,"rejected sample # %d (seekpos=%u < samplestart=%u)\n",t,q->seekpos,samplestart);
#endif
				_mm_errno = MMERR_LOADING_SAMPLEINFO;
				return 0;
			}
			/* Some .STMs seem to rely on allowing truncated samples... */
			if(q->seekpos>=sampleend) {
#ifdef MIKMOD_DEBUG
				fprintf(stderr,"truncating sample # %d from length %u to 0\n",t,q->length);
#endif
				q->seekpos = q->length = 0;
			} else if(q->seekpos+q->length>sampleend) {
#ifdef MIKMOD_DEBUG
				fprintf(stderr,"truncating sample # %d from length %u to %u\n",t,q->length,sampleend - q->seekpos);
#endif
				q->length = sampleend - q->seekpos;
			}
		}
		else
			q->seekpos = 0;

		/* contrary to the STM specs, sample data is signed */
		q->flags = SF_SIGNED;

		if(q->loopend && q->loopend != 0xffff && q->loopstart < q->length) {
			q->flags|=SF_LOOP;
			if (q->loopend > q->length)
				q->loopend = q->length;
		}
		else
			q->loopstart = q->loopend = 0;
	}
	return 1;
}

static CHAR *STM_LoadTitle(void)
{
	CHAR s[20];

	_mm_fseek(modreader,0,SEEK_SET);
	if(!_mm_read_UBYTES(s,20,modreader)) return NULL;

	return(DupStr(s,20,1));
}

/*========== Loader information */

MIKMODAPI MLOADER load_stm={
	NULL,
	"STM",
	"STM (Scream Tracker)",
	STM_Init,
	STM_Test,
	STM_Load,
	STM_Cleanup,
	STM_LoadTitle
};

/* ex:set ts=4: */
