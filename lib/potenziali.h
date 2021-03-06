#include <math.h>


//===============================================================================================================================
// PROTOTIPI
//===============================================================================================================================

double potenziali_lennard(double x);						// Funzione che restituisce la V(x)

double potenziali_armonico(double x);						// Potenziale armonico

double potenziali_sistema(struct PARTICLE* sys, double (*Vij)(double));		// Potenziale totale di un sistema


//===============================================================================================================================
// DEFINIZIONI
//===============================================================================================================================

// Funzione che restituisce la V(x) di Lennard Jones
double potenziali_lennard(double x)
{
	return 4.0*(pow(x,-12)-pow(x,-6));
}

// Funzione che restituisce la V(x) di un potenziale armonico
double potenziali_armonico(double x)
{
	return 0.5*x*x;
}

// Calcola il potenziale totale di un sistema di particelle interagenti tra loro con potenziale Vij
double potenziali_sistema(struct PARTICLE* sys, double (*Vij)(double))
{
	struct VEC_3D d;
	double potenziale, dmod;
	int i,j;

	potenziale=0;
	for(i=1; i<GLOBAL_N_PARTICLES; i++)
		for (j=i+1; j<GLOBAL_N_PARTICLES; j++)
		{
			d.x=sys[j].pos.x-sys[i].pos.x;
			d.y=sys[j].pos.y-sys[i].pos.y;
			d.z=sys[j].pos.z-sys[i].pos.z;
			dmod=pow(d.x*d.x + d.y*d.y + d.z*d.z,0.5);
			potenziale+=Vij(dmod);
	}	

	return potenziale;
}
