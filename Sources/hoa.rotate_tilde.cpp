/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco, Thomas Le Meur & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "../hoa.library.h"
#include "../ThirdParty/HoaLibrary/Sources/Hoa.hpp"
using namespace hoa;

typedef struct _hoa_rotate
{
    t_edspobj               f_obj;
    Rotate<Hoa2d, t_sample>*f_rotate;
    t_sample*               f_ins;
    t_sample*               f_outs;
    
} t_hoa_rotate;

static t_eclass *hoa_rotate_class;

static void *hoa_rotate_new(t_symbol *s, long argc, t_atom *argv)
{
    int	order = 4;
    t_hoa_rotate *x = (t_hoa_rotate *)eobj_new(hoa_rotate_class);
    
	if (x)
	{
		if(atom_gettype(argv) == A_LONG)
			order = pd_clip_min(atom_getlong(argv), 1);
		
		x->f_rotate = new Rotate<Hoa2d, t_sample>(order);
		
		eobj_dspsetup(x, x->f_rotate->getNumberOfHarmonics() + 1, x->f_rotate->getNumberOfHarmonics());
        
		x->f_ins = new t_float[x->f_rotate->getNumberOfHarmonics() * HOA_MAXBLKSIZE];
        x->f_outs = new t_float[x->f_rotate->getNumberOfHarmonics() * HOA_MAXBLKSIZE];
	}
    
	return (x);
}

static void hoa_rotate_float(t_hoa_rotate *x, float f)
{
    x->f_rotate->setYaw(f);
}

static void hoa_rotate_perform(t_hoa_rotate *x, t_object *dsp64, t_sample **ins, long numins, t_sample **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    for(long i = 0; i < numouts; i++)
    {
        cblas_scopy(sampleframes, ins[i], 1, x->f_ins+i, numouts);
    }
	for(long i = 0; i < sampleframes; i++)
    {
        x->f_rotate->setYaw(ins[numouts][i]);
        x->f_rotate->process(x->f_ins + numouts * i, x->f_outs + numouts * i);
    }
    for(long i = 0; i < numouts; i++)
    {
        cblas_scopy(sampleframes, x->f_outs+i, numouts, outs[i], 1);
    }
}

static void hoa_rotate_perform_offset(t_hoa_rotate *x, t_object *dsp64, t_sample **ins, long numins, t_sample **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    for(long i = 0; i < numouts; i++)
    {
        cblas_scopy(sampleframes, ins[i], 1, x->f_ins+i, numouts);
    }
	for(long i = 0; i < sampleframes; i++)
    {
        x->f_rotate->process(x->f_ins + numouts * i, x->f_outs + numouts * i);
    }
    for(long i = 0; i < numouts; i++)
    {
        cblas_scopy(sampleframes, x->f_outs+i, numouts, outs[i], 1);
    }
}

static void hoa_rotate_dsp(t_hoa_rotate *x, t_object *dsp, short *count, double samplerate, long maxvectorsize, long flags)
{
    if(count[x->f_rotate->getNumberOfHarmonics()])
        object_method(dsp, gensym("dsp_add"), x, (method)hoa_rotate_perform, 0, NULL);
    else
        object_method(dsp, gensym("dsp_add"), x, (method)hoa_rotate_perform_offset, 0, NULL);
}

static void hoa_rotate_free(t_hoa_rotate *x)
{
	eobj_dspfree(x);
	delete x->f_rotate;
    delete [] x->f_ins;
	delete [] x->f_outs;
}

extern "C" void setup_hoa0x2e2d0x2erotate_tilde(void)
{
    t_eclass* c;
    
    c = eclass_new("hoa.2d.rotate~",(method)hoa_rotate_new,(method)hoa_rotate_free, (short)sizeof(t_hoa_rotate), CLASS_NOINLET, A_GIMME, 0);
    class_addcreator((t_newmethod)hoa_rotate_new, gensym("hoa.rotate~"), A_GIMME, 0);
    
    eclass_dspinit(c);
    hoa_initclass(c);
    eclass_addmethod(c, (method)hoa_rotate_dsp,     "dsp",      A_CANT, 0);
    eclass_addmethod(c, (method)hoa_rotate_float,   "float",    A_FLOAT, 0);
    
    eclass_register(CLASS_OBJ, c);
    hoa_rotate_class = c;
}
