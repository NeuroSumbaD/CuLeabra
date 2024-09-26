#include "pattable.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

#include "strings.hpp"
#include "rand.hpp"
#include "emer.hpp"

// Function to parse the TSV header
pattable::ColumnInfo parseHeader(const std::string& headerLine, char delimitter) {
    // Split the header by tab
    std::vector<std::string> headers = strings::split(headerLine, delimitter);
    
    // Ignore "_H:" and "_D:" from emer formatting
    if (headers[0]=="_H:" || headers[0]=="_D:") {
        headers.erase(headers.begin());
    }

    // Map to store layer information
    pattable::ColumnInfo columnInfo;

    // Iterate over the remaining columns
    for (size_t i = 1; i < headers.size(); ++i) {
        std::string column = headers[i];

        // Find the position of '%' and '['
        size_t percentPos = column.find('%');
        size_t bracketPos = column.find('[');
        size_t angleBracketPos = column.find('<');

        if (percentPos != std::string::npos && bracketPos != std::string::npos) {
            std::string layerName = column.substr(percentPos + 1, bracketPos - percentPos - 1);

            // Parse layer index from header
            std::string tensorIndexString = column.substr(bracketPos + 1, column.find(']') - bracketPos - 1);
            std::vector<std::string> tensorIndicesString = strings::split(strings::split(tensorIndexString, ':')[1], ',');
            std::vector<int> tensorIndices(tensorIndicesString.size());
            int i = 0;
            for (std::string index: tensorIndicesString) {
                tensorIndices[i++] = std::stoi(index);
            }
            
            // Print or store the tensor dimensions if angle brackets are found
            if (angleBracketPos != std::string::npos) {
                std::string tensorDims = column.substr(angleBracketPos + 1, column.find('>') - angleBracketPos - 1);
                std::vector<std::string> tensorDimensionsString = strings::split(strings::split(tensorDims,':')[1], ',');
                std::vector<int> tensorDimensions(tensorDimensionsString.size());
                int i = 0;
                for (std::string index: tensorDimensionsString) {
                    tensorDimensions[i++] = std::stoi(index);
                }

                columnInfo.LayerShapes[layerName] = tensorDimensions;
            }

            // Store layer information
            columnInfo.LayerNames.push_back(layerName);
            columnInfo.LayerIndices.push_back(tensorIndices);
        } else {
            throw std::invalid_argument("ERROR: Something went wrong while parsing the header info");
        }
    }

    return columnInfo;
}

// Function to parse and process data rows
void processRow(pattable::Table *table, const std::string& rowLine, char delimitter) {
    std::vector<std::string> columns = strings::split(rowLine, delimitter);

    // Ignore "_H:" and "_D:" from emer formatting
    if (columns[0]=="_H:" || columns[0]=="_D:") {
        columns.erase(columns.begin());
    }

    // First column contains the event name
    std::string eventName = columns[0];
    table->events[eventName] = pattable::Event();
    table->eventNames.push_back(eventName);

    // Remaining columns contain activity data for layers
    std::string previousLayer = "";
    for (size_t col = 1; col < columns.size(); ++col) {
        float activityValue = std::stof(columns[col]);

        std::string layerName = table->info.LayerNames[col-1];
        if (layerName != previousLayer) { // initialize pattern tensor
            std::vector<int> layerShape = table->info.LayerShapes[layerName];
            table->events[eventName][layerName] = new tensor::Tensor<float>(layerShape);
            if (layerShape.size() == 2) {
                table->events[eventName][layerName]->SetShape(layerShape, emer::LayerDimNames2D);
            } else if (layerShape.size() == 4) {
                table->events[eventName][layerName]->SetShape(layerShape, emer::LayerDimNames4D);
            }
        }
        previousLayer = layerName;

        int tensorIndex = table->events[eventName][layerName]->Shp.Offset(table->info.LayerIndices[col-1]);
        table->events[eventName][layerName]->SetValue(tensorIndex, activityValue);
        // std::cout << "Activity: " << activityValue << " ";
    }
}

//Table is created by reading from a CSV
pattable::Table::Table(std::string fileName): MetaData(), events(), permutation(){
    ReadFile(fileName);
}

void pattable::Table::ReadFile(std::string fileName) {
    std::ifstream file(fileName);
    if (file.fail()) {
        throw std::runtime_error("ERROR: No file with name " + fileName + " found in the current directory.");
    }

    std::string line;

    char delimitter;
    if (fileName.find(".tsv")!=std::string::npos) {
        delimitter = '\t';
    } else if (fileName.find(".csv")!=std::string::npos) {
        delimitter = ',';
    }

    // Read the first line (header)
    if (std::getline(file, line)) {
        info = parseHeader(line, delimitter);
    }

    // Iterate over the remaining rows (after the header)
    while (std::getline(file, line)) {
        processRow(this, line, delimitter);
    }

    // TODO: Allow the permutation to be regenerated, overwritten, or not used
    uint numEvents = eventNames.size();
    permutation = rands::Perm(numEvents); //TODO: CHECK THIS FUNCTION (MULTIPLE ZEROS!!!)
}

// Returns a Tensor Pattern corresponding to the event
tensor::Tensor<float> *pattable::Table::GetPattern(std::string name) {
    if (EventIndex > eventNames.size()) {
        EventIndex = 0;
    }

    std::string eventName = eventNames[permutation[EventIndex]];
    tensor::Tensor<float> *pattern = events[eventName][name];

    if (numLayers == 0) { 
        numLayers = events[eventName].size();
        // increment once all the layer patterns have been applied
        EventIndex++;
    } else {
        numLayers--;
    }

    return pattern;
}
