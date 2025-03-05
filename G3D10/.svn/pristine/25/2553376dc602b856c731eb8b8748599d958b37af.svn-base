/**
  \file G3D-base.lib/include/G3D-base/PrefixTree.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/ReferenceCount.h"

namespace G3D {
class PrefixTree : public ReferenceCountedObject {
protected:
    /** Number of leaf nodes */
    int                             m_size = 0;
    const String                    m_value;        
    Array<shared_ptr<PrefixTree>>   m_children;

    /** Mutates array, omitting elements that are the empty string */
    static void rejectEmptyString(Array<String>& elements);

    static void compactSplit(const String& s, Array<String>& result);
                
    /** Modifies input array */
    static void compactJoin(Array<String>& elements, String& result);

    /** Perform input clean up on string s while converting to component array*/
    void fillComponents(String s, Array<String>& components);
        
    const shared_ptr<PrefixTree> childNodeWithPrefix(const String& s);

    bool canHaveChildren();

    /** If set to true, then we will treat special characters as spaces when creating PrefixTree*/
    bool m_cleanUpInput;

    /** ANSI "item separator" */
    static constexpr char DELIMITER = '\31';

public:

    PrefixTree(const String& value = "");
    virtual ~PrefixTree();
        
    const String& value() const { return m_value; }
    const Array<shared_ptr<PrefixTree>>& children() const { return m_children; }

    void insert(String s);
    bool contains(const String& s);
    /**
      If the node is a leaf, then its value is the full inserted value.
      Roughly, the result of joining all the prefixes on the path to the
      leaf; specifically, the String that was passed to PrefixTree::insert()
    */
    bool isLeaf() const { return m_children.size() == 0; };
    String getPathToBranch(shared_ptr<PrefixTree>& branchPoint);
    int size() const { return m_size; }
        
    static shared_ptr<PrefixTree> create( const String& s = "", bool cleanUpInput = false) {
        shared_ptr<PrefixTree> tree = createShared<PrefixTree>(s);
        tree->m_cleanUpInput = cleanUpInput;
        return tree;
    }

    template <class T> // Generic to allow both GuiText, String
    static shared_ptr<PrefixTree> create(const Array<T>& elements, bool cleanUpInput = false) {
        const shared_ptr<PrefixTree>& tree = PrefixTree::create(String(1, DELIMITER),cleanUpInput);
        for (const String& s : elements) {
            tree->insert(s);
        }
        return tree;
    }
};
}
