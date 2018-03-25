#include <enzymes.h>

#include <yats/pipeline.h>

using namespace  yats;

int main(int /*argc*/, char* /*argv*/ [])
{
    pipeline p;

    auto citrate_synthase_cfg = p.add<citrate_synthase>();
    auto aconitase1_cfg = p.add<aconitase1>();
    auto aconitase2_cfg = p.add<aconitase2>();
    auto isocitrate_dehydrogenase1_cfg = p.add<isocitrate_dehydrogenase1>();
    auto isocitrate_dehydrogenase2_cfg = p.add<isocitrate_dehydrogenase2>();
    auto alpha_ketoglutarate_dehydrogenase_cfg = p.add<alpha_ketoglutarate_dehydrogenase>();
    auto succinyl_co_a_synthetase_cfg = p.add<succinyl_co_a_synthetase>();
    auto succinate_dehydrogenase_cfg = p.add<succinate_dehydrogenase>();
    auto fumarase_cfg = p.add<fumarase>();
    auto malate_dehydrogenase_cfg = p.add<malate_dehydrogenase>();

    citrate_synthase_cfg->output<"Citrate"_id>() >> aconitase1_cfg->input<"Citrate"_id>();
    aconitase1_cfg->output<"Aconitate"_id>() >> aconitase2_cfg->input<"Aconitate"_id>();
    aconitase2_cfg->output<"Isocitrate"_id>() >> isocitrate_dehydrogenase1_cfg->input<"Isocitrate"_id>();
    isocitrate_dehydrogenase1_cfg->output<"Oxalosuccinate"_id>() >> isocitrate_dehydrogenase2_cfg->input<"Oxalosuccinate"_id>();
    isocitrate_dehydrogenase2_cfg->output<"aKetoglutarate"_id>() >> alpha_ketoglutarate_dehydrogenase_cfg->input<"aKetoglutarate"_id>();
    alpha_ketoglutarate_dehydrogenase_cfg->output<"Succinyl_CoA"_id>() >> succinyl_co_a_synthetase_cfg->input<"Succinyl_CoA"_id>();
    succinyl_co_a_synthetase_cfg->output<"Succinate"_id>() >> succinate_dehydrogenase_cfg->input<"Succinate"_id>();
    succinate_dehydrogenase_cfg->output<"Fumarate"_id>() >> fumarase_cfg->input<"Fumarate"_id>();
    fumarase_cfg->output<"Malate"_id>() >> malate_dehydrogenase_cfg->input<"Malate"_id>();
    malate_dehydrogenase_cfg->output<"Oxaloacetate"_id>() >> citrate_synthase_cfg->input<"Oxaloacetate"_id>();
    
    return 0;
}
