/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.8
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.imebra;

public class DicomDirEntry {
  private transient long swigCPtr;
  protected transient boolean swigCMemOwn;

  protected DicomDirEntry(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(DicomDirEntry obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        imebraJNI.delete_DicomDirEntry(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public DataSet getEntryDataSet() {
    long cPtr = imebraJNI.DicomDirEntry_getEntryDataSet(swigCPtr, this);
    return (cPtr == 0) ? null : new DataSet(cPtr, true);
  }

  public DicomDirEntry getNextEntry() {
    long cPtr = imebraJNI.DicomDirEntry_getNextEntry(swigCPtr, this);
    return (cPtr == 0) ? null : new DicomDirEntry(cPtr, true);
  }

  public DicomDirEntry getFirstChildEntry() {
    long cPtr = imebraJNI.DicomDirEntry_getFirstChildEntry(swigCPtr, this);
    return (cPtr == 0) ? null : new DicomDirEntry(cPtr, true);
  }

  public void setNextEntry(DicomDirEntry nextEntry) {
    imebraJNI.DicomDirEntry_setNextEntry(swigCPtr, this, DicomDirEntry.getCPtr(nextEntry), nextEntry);
  }

  public void setFirstChildEntry(DicomDirEntry firstChildEntry) {
    imebraJNI.DicomDirEntry_setFirstChildEntry(swigCPtr, this, DicomDirEntry.getCPtr(firstChildEntry), firstChildEntry);
  }

  public FileParts getFileParts() {
    return new FileParts(imebraJNI.DicomDirEntry_getFileParts(swigCPtr, this), true);
  }

  public void setFileParts(FileParts fileParts) {
    imebraJNI.DicomDirEntry_setFileParts(swigCPtr, this, FileParts.getCPtr(fileParts), fileParts);
  }

  public directoryRecordType_t getType() {
    return directoryRecordType_t.swigToEnum(imebraJNI.DicomDirEntry_getType(swigCPtr, this));
  }

  public String getTypeString() {
    return imebraJNI.DicomDirEntry_getTypeString(swigCPtr, this);
  }

}
