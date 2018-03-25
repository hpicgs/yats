/**
* This example to demonstrates the use of the function
* pipeline.save_to_file.
* 
* For this example the citric acid cycle 
* (see: https://en.wikipedia.org/wiki/Citric_acid_cycle)
* has been modelled as a pipeline. Not all inputs and
* outputs of the tasks (enzymes) are connected. These
* would have to be connected to external nodes. For this
* biological example, task free inputs are substrates which
* diffuse through the cell. Not connected outputs are
* substrates released back to the cell.
* 
* pipeline.save_to_file exports the pipeline as a text file.
* This text file can be converted into a nice graph using
* the command line tool Graphviz DOT (https://www.graphviz.org/)
* and the command: dot -Tpng TextFileName -o PictureName.png
*/

#include <cstdio>
#include <iostream>
#include <string>

#include <enzymes.h>

#include <yats/pipeline.h>

using namespace  yats;

int main(int /*argc*/, char* /*argv*/ [])
{
    std::cout << "This example to demonstrates the use of the function" << std::endl;
    std::cout << "pipeline.save_to_file." << std::endl;
    std::cout << std::endl;
    std::cout << "For this example the citric acid cycle" << std::endl;
    std::cout << "(see: https://en.wikipedia.org/wiki/Citric_acid_cycle)" << std::endl;
    std::cout << "has been modelled as a pipeline. Not all inputs and" << std::endl;
    std::cout << "outputs of the tasks (enzymes) are connected. These" << std::endl;
    std::cout << "would have to be connected to external nodes. For this" << std::endl;
    std::cout << "biological example, task free inputs are substrates which" << std::endl;
    std::cout << "diffuse through the cell. Not connected outputs are" << std::endl;
    std::cout << "substrates released back to the cell." << std::endl;
    std::cout << std::endl;
    std::cout << "pipeline.save_to_file exports the pipeline as a text file." << std::endl;
    std::cout << "This text file can be converted into a nice graph using" << std::endl;
    std::cout << "the command line tool Graphviz DOT (https://www.graphviz.org/)" << std::endl;
    std::cout << "and the command: dot -Tpng TextFileName -o PictureName.png" << std::endl;

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
    
    std::string filename;

    std::cout << std::endl;
    std::cout << "Please input filename to export pipeline to [citric_acid_cycle.txt]: ";
    std::getline(std::cin, filename);
    
    if (filename.empty())
    {
        filename = "citric_acid_cycle.txt";
    }

    p.save_to_file(filename);

    std::cout << std::endl;
    std::cout << "Pipeline exported to " << filename << '.' << std::endl;

    std::cout << std::endl;
    std::cout << "Execute dot on " << filename << " to create a picture of the pipeline." << std::endl;

    std::cout << std::endl;
    std::cout << "Do you want to remove " << filename << "? (Y/[N]): ";
    std::string input;
    std::getline(std::cin, input);
    if (input == "Y" || input == "y")
    {
        std::remove(filename.c_str());
        std::cout << filename << " removed." << std::endl;
    }

    return 0;
}
