/**
 * Defines all enzymes of the citric acid cycle.
 * See: https://en.wikipedia.org/wiki/Citric_acid_cycle
 */

#pragma once

#include <compounds.h>

#include <yats/identifier.h>
#include <yats/slot.h>

using namespace yats;

class aconitase1
{
public:
    output_bundle<slot<aconitate, "Aconitate"_id>, slot<water, "Water"_id>> run(slot<citrate, "Citrate"_id>)
    {
        const aconitate aconitate;
        const water water;
        return { aconitate, water };
    }
};

class aconitase2
{
public:
    slot<isocitrate, "Isocitrate"_id> run(slot<aconitate, "Aconitate"_id>, slot<water, "Water"_id>)
    {
        const isocitrate isocitrate;
        return isocitrate;
    }
};

class alpha_ketoglutarate_dehydrogenase
{
public:
    output_bundle<slot<succinyl_co_a, "Succinyl_CoA"_id>, slot<nadh, "NADH"_id>,
    slot<hydrogen, "H"_id>, slot<carbon_dioxide, "COO"_id>> run(slot<alpha_ketoglutarate, "aKetoglutarate"_id>,
        slot<nad, "NAD+"_id>, slot<coenzyme_a, "CoA_SH"_id>)
    {
        const succinyl_co_a succinyl_co_a;
        const nadh nadh;
        const hydrogen h;
        const carbon_dioxide co2;
        return { succinyl_co_a, nadh, h, co2 };
    }
};


class citrate_synthase
{
public:
    output_bundle<slot<citrate, "Citrate"_id>, slot<coenzyme_a, "CoA_SH"_id>> run(slot<oxaloacetate, "Oxaloacetate"_id>,
        slot<acetyl_co_a, "Acetyl CoA"_id>, slot<water, "Water"_id>)
    {
        const citrate citrate;
        const coenzyme_a co_a_sh;
        return { citrate, co_a_sh };
    }
};

class fumarase
{
public:
    slot<malate, "Malate"_id> run(slot<fumarate, "Fumarate"_id>, slot<water, "Water"_id>)
    {
        const malate malate;
        return malate;
    }
};

class isocitrate_dehydrogenase1
{
public:
    output_bundle<slot<oxalosuccinate, "Oxalosuccinate"_id>, slot<nadh, "NADH"_id>, slot<hydrogen, "H"_id>> run(slot<isocitrate, "Isocitrate"_id>,
        slot<nad, "NAD+"_id>)
    {
        const oxalosuccinate oxalosuccinate;
        const nadh nadh;
        const hydrogen h;
        return { oxalosuccinate, nadh, h };
    }
};

class isocitrate_dehydrogenase2
{
public:
    output_bundle<slot<alpha_ketoglutarate, "aKetoglutarate"_id>, slot<carbon_dioxide, "COO"_id>> run(slot<oxalosuccinate, "Oxalosuccinate"_id>)
    {
        const alpha_ketoglutarate alpha_ketoglutarate;
        const carbon_dioxide co2;
        return { alpha_ketoglutarate, co2 };
    }
};

class malate_dehydrogenase
{
public:
    output_bundle<slot<oxaloacetate, "Oxaloacetate"_id>, slot<nadh, "NADH"_id>, slot<hydrogen, "H"_id>> run(slot<malate, "Malate"_id>,
        slot<nad, "NAD+"_id>)
    {
        const oxaloacetate oxaloacetate;
        const nadh nadh;
        const hydrogen h;
        return { oxaloacetate, nadh, h };
    }
};

class succinate_dehydrogenase
{
public:
    output_bundle<slot<fumarate, "Fumarate"_id>, slot<ubiquinol, "Ubiquinol"_id>> run(slot<succinate, "Succinate"_id>,
        slot<ubiquinone, "Ubiquinone"_id>)
    {
        const fumarate fumarate;
        const ubiquinol qh2;
        return { fumarate, qh2 };
    }
};

class succinyl_co_a_synthetase
{
public:
    output_bundle<slot<succinate, "Succinate"_id>, slot<coenzyme_a, "CoA_SH"_id>, slot<gtp, "GTP"_id>> run(slot<succinyl_co_a, "Succinyl_CoA"_id>,
        slot<gdp, "GDP"_id>, slot<phosphate, "P"_id>)
    {
        const succinate succinate;
        const coenzyme_a coenzyme_a;
        const gtp gtp;
        return { succinate, coenzyme_a, gtp };
    }
};
