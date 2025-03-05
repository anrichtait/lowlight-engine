/**
  \file G3D-base.lib/source/PrefixTree.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/PrefixTree.h"
#include <regex>

namespace G3D {
    
PrefixTree::PrefixTree(const String& s) : m_value(s) {}

PrefixTree::~PrefixTree() {}

void PrefixTree::rejectEmptyString(Array<String>& elements) {
    int i = 0;
    while (i < elements.size()) {
        if (elements[i].empty()) {
            elements.remove(i);
        } else {
            ++i;
        }
    }
}


void PrefixTree::compactSplit(const String& s, Array<String>& result) {
    stringSplit(s, DELIMITER, result);
    rejectEmptyString(result);
}
                

void PrefixTree::compactJoin(Array<String>& elements, String& result) {
    rejectEmptyString(elements);
    result = stringJoin(elements, "");
    // if joined component is surronded by parenthesis remove those
    const static std::regex parenBreakPattern("^([[:space:]]*)\\((.*)\\)$", std::regex::ECMAScript);
    const static std::string parenReplacePattern = std::string("$1$2");
    const std::string& tparen = regex_replace(result.c_str(), parenBreakPattern, parenReplacePattern);
    result = String(tparen);
}


bool PrefixTree::canHaveChildren() {
    return (m_children.size() > 0) || m_value.empty();
}


const shared_ptr<PrefixTree> PrefixTree::childNodeWithPrefix(const String& s) {
   for (shared_ptr<PrefixTree>& child : m_children) {
        if (child->value() == s && child->canHaveChildren()) {
            return child;
        }
    }
    return nullptr;
}

void PrefixTree::fillComponents(String s, Array<String>& components){
    //split on delimiter first for spaces
    const static std::regex spaceBreakPattern(" ", std::regex::ECMAScript);
    const static std::string spaceReplacePattern = DELIMITER + std::string(" ");
    const std::string& tspace = regex_replace(s.c_str(), spaceBreakPattern, spaceReplacePattern);

    if (m_cleanUpInput){
         // Introduce splitting points on non-spaces
        const static std::regex cleanBreakPattern("(::|\\\\|/|\\.)", std::regex::ECMAScript);
        const static std::string cleanReplacePattern = std::string("$1") + DELIMITER;
        const std::string& tclean = regex_replace(tspace.c_str(), cleanBreakPattern, cleanReplacePattern);

        compactSplit(String(tclean), components);
    }
}

void PrefixTree::insert(String s) {
    Array<String> components;
    fillComponents(s,components);

    // Iterate to *parent* of leaf of existing prefix tree
    // The leaves store the original representation of the element, preserving
    // whitespace, so we should never alter them and instead stop at the parent 
    shared_ptr<PrefixTree> finger = dynamic_pointer_cast<PrefixTree>(shared_from_this());
    ++finger->m_size;
    int i = 0;
    while (i < components.size()) {
        // Check if any of the finger's children contain the next component
        const shared_ptr<PrefixTree>& next = finger->childNodeWithPrefix(components[i]);
        
        // If not, then we are at the insertion point
        if (isNull(next)) {
            break;
        }

        // If so, continue traversing the prefix tree
        finger = next;
        ++finger->m_size;
        ++i;
    }

    // Add nodes as necessary, starting at the insertion point
    while (i < components.size()) {
        const String& component(components[i]);

        const shared_ptr<PrefixTree>& next = PrefixTree::create(component, m_cleanUpInput);
        finger->m_children.append(next);
        finger = next;
        ++finger->m_size;

        ++i;
    }

    // Add leaf node with precise value of string
    finger->m_children.append(std::make_shared<PrefixTree>(s));
}


bool PrefixTree::contains(const String& s) {
    Array<String> components;
    fillComponents(s,components);

    // Iterate to *parent* of leaf of existing prefix tree
    // The leaves store the original representation of the element, preserving
    // whitespace, so we should never alter them and instead stop at the parent 
    shared_ptr<PrefixTree> finger = dynamic_pointer_cast<PrefixTree>(shared_from_this());
    for (const String& component : components) {
        // Check if any of the finger's children contain the next component
        const shared_ptr<PrefixTree>& next = finger->childNodeWithPrefix(component);

        if (! next) {
            return false;
        }
        finger = next;
    }
    
    // Check that the leaves that extend 
    for (const shared_ptr<PrefixTree>& node : finger->m_children) {
        if (node->isLeaf() && (node->value() == s)) {
            return true;
        } 
    }

    return false;
}


String PrefixTree::getPathToBranch(shared_ptr<PrefixTree>& branchPoint) {
    shared_ptr<PrefixTree> finger = dynamic_pointer_cast<PrefixTree>(shared_from_this());       
    Array<String> pathParts;
    while (finger->m_children.size() == 1) {
        pathParts.append(finger->value());
        finger = finger->m_children[0];
    }

    // Omit leaf nodes from the string
    branchPoint = finger;
    String result;
    if (finger->m_children.size() > 0) {
        pathParts.append(finger->value());
        compactJoin(pathParts, result);
        return result + "   >";
    } else {
        compactJoin(pathParts, result);
        return result;
    }
}




}
