#include <math.h>
#include <stdio.h>
#include "vario.h"


//=====================================================================================================================
// PROTOTIPI
//=====================================================================================================================

int		dinamica_inizializza(struct PARTICLE*);
struct PARTICLE	dinamica_evolvi_particle_verlet(struct PARTICLE*, int ,double (*)(double));
int		dinamica_evolvi_sistema_verlet(struct PARTICLE*, double (*)(double));
struct VEC_3D	dinamica_box_boundary(double, double, double);
struct VEC_3D	dinamica_calcoloforza(struct PARTICLE* sys, int i, double (*Vij)(double));
double		dinamica_calcolapressione(struct PARTICLE* sys, double (*Vij)(double));
double 		dinamica_calcolaenergia(struct PARTICLE* sys, double (*Vij)(double));
int 		dinamica_salvastato(char* pathfile, struct PARTICLE* sys, long int iter_done);
long int 	dinamica_caricastato(char* pathfile, struct PARTICLE* sys);


//=====================================================================================================================
// DEFINIZIONI
//=====================================================================================================================

//===============inizializza il sistema================================================================================
int dinamica_inizializza(struct PARTICLE* sys)
{				
	int i, j, k, count=0;
	double dL ; 	
	double sigma=pow(GLOBAL_THETA/GLOBAL_PARTICLE_MASS,0.5);						

	dL = (double)GLOBAL_L_BOX / (double)GLOBAL_DIVISIONS;		// passo reticolo

	for (i=0; i<GLOBAL_DIVISIONS; i++)
		for (j=0; j<GLOBAL_DIVISIONS; j++)
			for (k=0; k<GLOBAL_DIVISIONS; k++)
			{
				sys[count].mass = GLOBAL_PARTICLE_MASS;
				
				sys[count].pos = dinamica_box_boundary(((double)i+0.5)*dL,((double)j+0.5)*dL,((double)k+0.5)*dL);	//centrato

				sys[count].vel.x = vario_rnd_gauss() * sigma;
				sys[count].vel.y = vario_rnd_gauss() * sigma;
				sys[count].vel.z = vario_rnd_gauss() * sigma;

				sys[count].posold.x = sys[count].pos.x-sys[count].vel.x*GLOBAL_DT;
				sys[count].posold.y = sys[count].pos.y-sys[count].vel.y*GLOBAL_DT;
				sys[count].posold.z = sys[count].pos.z-sys[count].vel.z*GLOBAL_DT;
				
				count++;
			}
	return 0;
}

//===============calcolo posizione con metodo Verlet====================================================================
struct PARTICLE dinamica_evolvi_particle_verlet(struct PARTICLE* sys, int i, double (*Vij)(double))
{	
	struct VEC_3D f;
	struct VEC_3D posnew;
	struct PARTICLE p;

	f = dinamica_calcoloforza(sys, i, Vij);

	p=sys[i];
	posnew.x=2*p.pos.x-p.posold.x+f.x*GLOBAL_DT*GLOBAL_DT/p.mass;
	posnew.y=2*p.pos.y-p.posold.y+f.y*GLOBAL_DT*GLOBAL_DT/p.mass;
	posnew.z=2*p.pos.z-p.posold.z+f.z*GLOBAL_DT*GLOBAL_DT/p.mass;
	posnew=dinamica_box_boundary(posnew.x, posnew.y, posnew.z);
					
	p.posold=p.pos;
	p.pos=posnew;
	
	return p;
}

int dinamica_evolvi_sistema_verlet(struct PARTICLE* sys, double (*Vij)(double))
{
	struct PARTICLE temp[GLOBAL_N_PARTICLES];
	int i;

	for (i=0; i<GLOBAL_N_PARTICLES; i++)
	{
		temp[i]=dinamica_evolvi_particle_verlet(sys, i, Vij);
	}

	for (i=0; i<GLOBAL_N_PARTICLES; i++)
	{
		sys[i]=temp[i];
	}

	return 0;
}


//===============condizioni al contorno scatola=========================================================================
struct VEC_3D dinamica_box_boundary(double x, double y, double z)
{
	struct VEC_3D a;

	a.x=x-GLOBAL_L_BOX*rint(x/((double)GLOBAL_L_BOX));
	a.y=y-GLOBAL_L_BOX*rint(y/((double)GLOBAL_L_BOX));
	a.z=z-GLOBAL_L_BOX*rint(z/((double)GLOBAL_L_BOX));

	return a;
}


//===============calcola la forza agente sulla particella i-esima=======================================================
struct VEC_3D dinamica_calcoloforza(struct PARTICLE* sys, int i, double (*Vij)(double))
{
	int j;
	double h, rmod, fmod;
	struct VEC_3D f, r;
	f.x=0;
	f.y=0;
	f.z=0;
	h = 0.0001;

	for (j=0; j<GLOBAL_N_PARTICLES; j++)
		if (j!=i)
		{
			r.x=sys[j].pos.x-sys[i].pos.x;
			r.y=sys[j].pos.y-sys[i].pos.y;			
			r.z=sys[j].pos.z-sys[i].pos.z;
			r=dinamica_box_boundary(r.x, r.y, r.z);			
			
			rmod=pow(r.x*r.x+r.y*r.y+r.z*r.z,0.5);
			fmod = -(analisi_deriva_trepunti(rmod, h , Vij));

			f.x+=fmod*r.x/rmod;
			f.y+=fmod*r.y/rmod;
			f.z+=fmod*r.z/rmod;		
		}
	return f;
}


//===============calcola la pressione del gas ==========================================================================
double dinamica_calcolapressione(struct PARTICLE* sys, double (*Vij)(double))
{
	double w, h, rmod, wmod;
	int i, j;
	struct VEC_3D r;
	w = 0;
	h = 0.0001;
	for (i=0; i<GLOBAL_N_PARTICLES; i++)
	{
		for (j=i+1; j<GLOBAL_N_PARTICLES; j++)
		{
			r.x=sys[j].pos.x-sys[i].pos.x;
			r.y=sys[j].pos.y-sys[i].pos.y;			
			r.z=sys[j].pos.z-sys[i].pos.z;
			r=dinamica_box_boundary(r.x, r.y, r.z);			
			
			rmod=pow(r.x*r.x+r.y*r.y+r.z*r.z,0.5);
			
			wmod=-analisi_deriva_trepunti(rmod, h , Vij);
			
			w+=wmod*rmod;		
		}
	}
	return (((double)GLOBAL_N_PARTICLES*GLOBAL_THETA + w/3.0)/(GLOBAL_L_BOX*GLOBAL_L_BOX*GLOBAL_L_BOX));
}

double dinamica_calcolaenergia(struct PARTICLE* sys, double (*Vij)(double))
{
	double k=0, vquad;
	int i;
	for (i=0; i<GLOBAL_N_PARTICLES; i++)
	{
		vquad= ((sys[i].pos.x-sys[i].posold.x)*(sys[i].pos.x-sys[i].posold.x)+(sys[i].pos.y-sys[i].posold.y)*(sys[i].pos.y-sys[i].posold.y)+(sys[i].pos.z-sys[i].posold.z)*(sys[i].pos.z-sys[i].posold.z))/(GLOBAL_DT*GLOBAL_DT);
		k+= 0.5*sys[i].mass*vquad;
	}	
	return potenziali_sistema(sys, Vij)+k;
}

//===============salvataggio ultima configurazione e parametri=======================================================
int dinamica_salvastato(char* pathfile, struct PARTICLE* sys, long int iter_done)
{
	int i;
	FILE* file;
	
	file = fopen(pathfile, "w");

	if(!file) {fprintf(stderr,"ERRORE NELLA CREAZIONE DEL FILE %s!", pathfile); return -1;}

	if(!fprintf(file,"#DIVISIONS %d\n", GLOBAL_DIVISIONS)) 		{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	if(!fprintf(file,"#N_PARTICLES %d\n", GLOBAL_N_PARTICLES))	{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	if(!fprintf(file,"#L_BOX %lf\n", GLOBAL_L_BOX)) 		{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	if(!fprintf(file,"#DT %lf\n", GLOBAL_DT)) 			{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	if(!fprintf(file,"#PARTICLE_MASS %lf\n", GLOBAL_PARTICLE_MASS))	{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	if(!fprintf(file,"#ITER_DONE %ld\n", iter_done))		{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	if(!fprintf(file,"#ITER_TOT %d\n", GLOBAL_ITER_TOT))		{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	
	for (i=0; i<GLOBAL_N_PARTICLES; i++)
	{
		if(!fprintf(file,"%f %f %f %f %f %f %f %f %f\n", sys[i].pos.x, sys[i].pos.y, sys[i].pos.z, sys[i].posold.x, sys[i].posold.y, sys[i].posold.z, sys[i].vel.x, sys[i].vel.y, sys[i].vel.z)) 		{fprintf(stderr,"ERRORE NELLA SCRITTURA DEL FILE %s!", pathfile); return -1;}
	}

	fclose(file);
	
	return 0;
}


//===============salvataggio ultima configurazione e parametri=======================================================
long int dinamica_caricastato(char* pathfile, struct PARTICLE* sys)
{
	int i;
	
	FILE* file;
	file = fopen(pathfile, "r");

	if (!file) {fprintf(stderr,"ERRORE NELLA LETTURA DEL FILE %s!", pathfile); return -1;}

	/*fscanf(file,"%s %d\n",a,&b);
	fscanf(file,"%s %d\n",a,&b);
	fscanf(file,"%s %d\n",a,&b);
	fscanf(file,"%s %d\n",a,&b);
	fscanf(file,"%s %d\n",a,&b);
	fscanf(file,"%s %d\n", a, &iter_tot);*/
	
	for (i=0; i<GLOBAL_N_PARTICLES; i++)
	{
		if(!fscanf(file,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n", &sys[i].pos.x, &sys[i].pos.y, &sys[i].pos.z, &sys[i].posold.x, &sys[i].posold.y, &sys[i].posold.z, &sys[i].vel.x, &sys[i].vel.y, &sys[i].vel.z))  	{fprintf(stderr,"ERRORE NELLA LETTURA DEL FILE %s!", pathfile); return -1;};
		
	}
	
	fclose(file);
	return 0;
}


