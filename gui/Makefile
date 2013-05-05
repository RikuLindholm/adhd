JFLAGS = -g -cp .
JC = javac

CLASSES = \
	Connection.java \
	IconButton.java \
	FileMessage.java \
	SaveAsDialog.java \
	Gui.java

.SUFFIXES: .java .class

.java.class:
	$(JC) $(JFLAGS) $*.java

all: classes

classes: $(CLASSES:.java=.class)
