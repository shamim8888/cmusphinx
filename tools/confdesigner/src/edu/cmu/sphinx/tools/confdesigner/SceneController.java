package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.tools.executor.ExecutorListener;
import edu.cmu.sphinx.tools.executor.Executable;
import edu.cmu.sphinx.util.props.*;
import org.netbeans.api.visual.widget.EventProcessingType;

import javax.swing.*;
import java.awt.*;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.beans.Introspector;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.List;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class SceneController implements ConfigurationChangeListener {

    private ConfigurationManager cm;
    private ConfigScene scene;


    public final String LAYOUT_SUFFIX = ".layout";

    private List<ExecutorListener> executorListeners = new ArrayList<ExecutorListener>();

    private boolean isChanged, doActiveFeedback = true;


    public SceneController(ConfigScene scene) {
        scene.acceptProvider.setController(this);
        setScene(scene);
        setCm(new ConfigurationManager());
    }


    public void setScene(final ConfigScene scene) {
        assert scene != null;
        this.scene = scene;
    }


    public void save(File cmLocation) {
        ConfigurationManagerUtils.save(cm, cmLocation);
        SceneSerializer.saveLayout(getScene(), new File(cmLocation.getAbsolutePath() + LAYOUT_SUFFIX));
    }


    public ConfigurationManager getCm() {
        return cm;
    }


    public void setCm(ConfigurationManager cm) {
        assert cm != null;

        if (this.cm != null) {
            for (ConfEdge edge : scene.getEdges().toArray(new ConfEdge[]{})) {
                scene.removeEdge(edge);
            }

            for (ConfNode confNode : scene.getNodes().toArray(new ConfNode[]{})) {
                scene.removeNode(confNode);
            }

            scene.validate();
        }

        this.cm = cm;

        // remove all content from the graph and build up a new one
        doActiveFeedback = false;
        new GraphLoader(this, scene).loadScene(cm, executorListeners);
        doActiveFeedback = true;

        isChanged = false;

        cm.addConfigurationChangeListener(new ConfigurationChangeListener() {

            public void configurationChanged(String configurableName, String propertyName, ConfigurationManager cm) {
                PropertySheet ps = cm.getPropertySheet(configurableName);

                // handle graph rerouting
                PropertySheet.PropertyType type = ps.getType(propertyName);
                ConfNode confNode = scene.findNodeByName(configurableName);


                if (type.equals(PropertySheet.PropertyType.COMP)) {
                    String propVal = (String) ps.getRaw(propertyName);
                    ConfPin pin = confNode.getPin(propertyName);

                    // remove the according edge

                    if (propVal == null) {
                        Collection<ConfEdge> confEdges = scene.findPinEdges(pin, false, true);
                        assert confEdges.size() <= 1 : "more than one connection to a property pin won't define anything meaningful";

                        if (!confEdges.isEmpty())
                            scene.removeEdge(confEdges.iterator().next());
                    } else {
                        // add a new graph connection
//                        ConfNode sourceNode = scene.findNodeByName(propVal);
//                        ConfPin sourcePin = sourceNode.getThisPin();
//
//                        scene.addEdge(new ConfEdge(sourcePin, pin));
                    }

                } else if (type.equals(PropertySheet.PropertyType.COMPLIST)) {
                    List<String> propList = (List<String>) ps.getRaw(propertyName);
//
                    // remove all edges which are not in the list
                    List<ConfPin> listPins = confNode.getListPins(propertyName);

                    for (ConfPin listPin : listPins) {
                        Collection<ConfEdge> pinEdges = scene.findPinEdges(listPin, false, true);
                        if (!pinEdges.isEmpty()) {
                            ConfEdge edge = pinEdges.iterator().next();
                            String instanceName = scene.getPinNode(edge.getSource()).getInstanceName();
                            if (!propList.contains(instanceName)) {
                                scene.removeEdge(edge);
                            }
                        }
                    }
                    scene.validate();

                    // ensure that there is at least one empty pin
                    while (GraphLoader.getUnusedListPins(confNode, propertyName, scene).size() > 1) {
                        List<ConfPin> unusedPins = GraphLoader.getUnusedListPins(confNode, propertyName, scene);

                        ConfPin remPin = unusedPins.iterator().next();
                        confNode.revoveListPin(remPin);
                        scene.removePin(remPin);
//                        scene.validate();
                    }

                    if (GraphLoader.getUnusedListPins(confNode, propertyName, scene).size() == 0) {
                        List<ConfPin> pins = confNode.getListPins(propertyName);
                        Class<? extends Configurable> classType = pins.iterator().next().getType();

                        confNode.addInputPin(scene, propertyName, pins.size() + 1, classType);
                    }

                    // ensure that listPins are numbered in increasing order
                    listPins = confNode.getListPins(propertyName);

                    for (int i = 0; i < listPins.size(); i++) {
                        ConfPin confPin = listPins.get(i);
                        confPin.setListPosition(i + 1);

                        PortWidget widget = (PortWidget) scene.findWidget(confPin);
                        widget.getLabelWidget().setLabel(confPin.toString());
                    }

                    assert GraphLoader.getUnusedListPins(confNode, propertyName, scene).size() == 1;
                }

                scene.validate();
            }


            public void componentAdded(ConfigurationManager cm, PropertySheet ps) {
                String configurableName = ps.getInstanceName();
                addNode(cm.getPropertySheet(configurableName), configurableName);
            }


            public void componentAdded(PropertySheet propertySheet, ConfigurationManager cm) {

            }


            public void componentRemoved(ConfigurationManager configurationManager, PropertySheet ps) {
                scene.removeNode((ConfNode) scene.findObject(scene.findWidgetByName(ps.getInstanceName())));
                scene.validate();

                for (ExecutorListener executorListener : executorListeners) {
                    executorListener.removedExecutor(ps);
                }
            }
        });

        scene.connectProvider.setCM(cm);
    }


    // add a new node which was not registered to the current configuration manager
    public String addNode(Class<? extends Configurable> confClass, String compName) {
        // create a new instance name if no name was given
        if (compName == null) {
            String[] strings = confClass.getName().split("[.]");
            compName = Introspector.decapitalize(strings[strings.length - 1]);
            if (cm.getComponentNames().contains(compName)) {
                int counter = 1;
                while (cm.getComponentNames().contains(compName + counter))
                    counter++;

                compName += counter;
            }
        }

        cm.addConfigurable(confClass, compName);

        return compName;
    }


    public ConfNode addNode(PropertySheet propSheet, String compName) {

        ConfNode node = new ConfNode(compName, propSheet);
        scene.addNode(node);
        createPropertyPins(node, scene);

        if (ConfigurationManagerUtils.isImplementingInterface(propSheet.getConfigurableClass(), Executable.class)) {
            for (ExecutorListener executorListener : executorListeners) {
                executorListener.addedExecutor(propSheet);
            }
        }

        return node;
    }


    public void createPropertyPins(ConfNode node, ConfigScene scene) {
        PropertySheet ps = node.getPropSheet();
        // create the connection pin which allows to connect this component to other components
        node.addOutputPin(scene, ps.getConfigurableClass());

        // create all pins
        for (String propName : ps.getRegisteredProperties()) {
            PropertySheet.PropertyType propType = ps.getType(propName);

            Class<? extends Configurable> type;
            try {
                if (propType == PropertySheet.PropertyType.COMP) {
                    type = ((S4Component) ps.getProperty(propName, S4Component.class).getAnnotation()).type();
                    node.addInputPin(scene, propName, type);

                } else if (propType == PropertySheet.PropertyType.COMPLIST) {
                    type = ((S4ComponentList) ps.getProperty(propName, S4ComponentList.class).getAnnotation()).type();
                    node.addInputPin(scene, propName, 1, type);
                }
            } catch (PropertyException e) {
                e.printStackTrace();
            }
        }
    }


    /** Adds a new listener. */
    public void addExecutorListener(ExecutorListener l) {
        if (l == null)
            return;

        executorListeners.add(l);
    }


    /** Removes a listener. */
    public void removeExecutorListener(ExecutorListener l) {
        if (l == null)
            return;

        executorListeners.remove(l);
    }


    public Component getView() {
        if (scene.getView() != null)
            return scene.getView();

        JComponent sceneView = scene.createView();

        sceneView.setFocusable(true);
        sceneView.setEnabled(true);
        scene.setKeyEventProcessingType(EventProcessingType.ALL_WIDGETS);

        sceneView.addKeyListener(new KeyAdapter() {

            public void keyPressed(KeyEvent e) {
                if (e.getKeyCode() == KeyEvent.VK_DELETE) {
                    List selObjectList = new ArrayList(scene.getSelectedObjects());
                    Set selectedObjects = scene.getSelectedObjects();
                    List selEdges = new ArrayList();

                    // select all edges of selected nodes
                    for (Object selectedObject : selObjectList) {

                        if (selectedObject instanceof ConfNode) {
                            ConfNode remNode = (ConfNode) selectedObject;
                            for (ConfPin pin : scene.getNodePins(remNode)) {
                                Collection<ConfEdge> pinEdges = scene.findPinEdges(pin, true, true);

                                for (ConfEdge pinEdge : pinEdges) {
                                    selEdges.add(pinEdge);
                                }
                            }
                        }
                    }

                    selObjectList.addAll(selEdges);
                    scene.setSelectedObjects(new HashSet<Object>(selObjectList));

                    // remove all selected edges
                    selectedObjects = scene.getSelectedObjects();
                    for (Object selectedObject : selectedObjects.toArray()) {
                        if (selectedObject instanceof ConfEdge) {
                            ConfEdge edge = (ConfEdge) selectedObject;

                            ConfPin pin = edge.getTarget();
                            PropertySheet ps = scene.getPinNode(pin).getPropSheet();
                            String propName = pin.getPropName();
                            try {

                                if (!pin.isListPin()) {
                                    ps.setComponent(propName, null, null);

                                } else {
                                    List<String> compList = (List<String>) ps.getRaw(pin.getPropName());
                                    compList.remove(scene.getPinNode(edge.getSource()).getInstanceName());

                                    ps.setComponentList(propName, compList, null);
                                }
                            } catch (PropertyException e1) {
                                e1.printStackTrace();
                            }
                        }
                    }

                    // remove all selected nodes
                    for (Object selectedObject : selectedObjects.toArray()) {
                        if (selectedObject instanceof ConfNode) {
                            cm.removeConfigurable(((ConfNode) selectedObject).getInstanceName());
                        }
                    }
                }
            }
        });

        return sceneView;
    }


    public void setCm(File cmLocation) {
        try {
            if (cmLocation == null)
                setCm(new ConfigurationManager());
            else {
                setCm(new ConfigurationManager(cmLocation.toURI().toURL()));

                File layoutFile = new File(cmLocation.getAbsolutePath() + LAYOUT_SUFFIX);
                if (layoutFile.isFile())
                    SceneSerializer.loadLayout(getScene(), layoutFile);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (PropertyException e) {
            e.printStackTrace();
        }
    }


    public boolean hasUnsavedChanges() {
        return isChanged;
    }


    public void configurationChanged(String configurableName, String propertyName, ConfigurationManager cm) {
        isChanged = true;
    }


    public void componentAdded(ConfigurationManager configurationManager, PropertySheet propertySheet) {
        isChanged = true;

    }


    public void componentRemoved(ConfigurationManager configurationManager, PropertySheet propertySheet) {
    }


    public ConfigScene getScene() {
        return scene;
    }
}
