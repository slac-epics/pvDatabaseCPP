/* pvDatabase.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2012.11.20
 */
#ifndef PVDATABASE_H
#define PVDATABASE_H

#include <list>
#include <map>
#include <deque>

#include <pv/pvAccess.h>
#include <pv/convert.h>

namespace epics { namespace pvDatabase { 

class PVRecord;
typedef std::tr1::shared_ptr<PVRecord> PVRecordPtr;
typedef std::map<epics::pvData::String,PVRecordPtr> PVRecordMap;

class PVRecordField;
typedef std::tr1::shared_ptr<PVRecordField> PVRecordFieldPtr;
typedef std::vector<PVRecordFieldPtr> PVRecordFieldPtrArray;
typedef std::tr1::shared_ptr<PVRecordFieldPtrArray> PVRecordFieldPtrArrayPtr;

class PVRecordStructure;
typedef std::tr1::shared_ptr<PVRecordStructure> PVRecordStructurePtr;

class PVRecordClient;
typedef std::tr1::shared_ptr<PVRecordClient> PVRecordClientPtr;

class PVListener;
typedef std::tr1::shared_ptr<PVListener> PVListenerPtr;

class PVDatabase;
typedef std::tr1::shared_ptr<PVDatabase> PVDatabasePtr;

/**
 * Base interface for a record.
 * @author mrk
 */
class PVRecord :
     public epics::pvData::Requester,
     public std::tr1::enable_shared_from_this<PVRecord>
{
public:
    POINTER_DEFINITIONS(PVRecord);

    /**
     * Virtual initialization method.
     * Must be implemented by derived classes.
     * This method <b>Must</b> call initPVRecord.
     * @return <b>true</b> for success and <b>false</b> for failure.
     */
    virtual bool init() {initPVRecord(); return true;}
    /**
     *  Must be implemented by derived classes.
     *  It is the method that makes a record smart.
     *  If it encounters errors it should raise alarms and/or
     *  call the <b>message</b> method provided by the base class.
     */
    virtual void process() {}
    /**
     * Creates a <b>dump</b> record, i.e. a record where process does nothing. 
     * @param recordName The name of the record, which is also the channelName.
     * @param pvStructure The top level structure.
     * @return A shared pointer to the newly created record.
     */
    static PVRecordPtr create(
        epics::pvData::String const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    /**
     * The Destructor. Must be virtual.
     */
    virtual ~PVRecord();
    /**
     *  Destroy the PVRecord. Release any resources used and 
     *  get rid of listeners and requesters.
     */
    virtual void destroy();
    /**
     * Get the name of the record.
     * @return The name.
     */
    epics::pvData::String getRecordName();
    /**
     * Get the top level PVStructure.
     * @return The shared pointer.
     */
    PVRecordStructurePtr getPVRecordStructure();
    /**
     * Find the PVRecordField for the PVField.
     * @param pvField The PVField.
     * @return The shared pointer to the PVRecordField.
     */
    PVRecordFieldPtr findPVRecordField(
        epics::pvData::PVFieldPtr const & pvField);
    /**
     * Add a requester to receive messages.
     * @param requester The requester.
     * @return <b>true</b> if requester was added.
     */
    bool addRequester(epics::pvData::RequesterPtr const & requester);
    /**
     * Remove a requester.
     * @param requester The requester.`
     * @return <b>true</b> if requester was removed.
     */
    bool removeRequester(epics::pvData::RequesterPtr const & requester);
    /**
     *  This is an inline method that locks the record.
     *  The record will automatically
     *   be unlocked when control leaves the block that has the call.
     */
    inline void lock_guard() { epics::pvData::Lock theLock(mutex); }
    /**
     * Lock the record.
     * Any code must lock while accessing a record.
     */
    void lock();
    /**
     * Unlock the record.
     */
    void unlock();
    /**
     * If <b>true</b> then just like <b>lock</b>.
     * If <b>false</b>client can not access record.
     * Code can try to simultaneously hold the lock for more than two records
     * by calling this method but must be willing to accept failure.
     * @return <b>true</b> if the record is locked.
     */
    bool tryLock();
    /**
     * A client that holds the lock for one record can lock one other record.
     * A client <b>must</b> not call this if the client already has the lock for
     * more then one record.
     *
     * @param otherRecord The other record to lock.
     */
    void lockOtherRecord(PVRecordPtr const & otherRecord);
    /**
     * Every client that accesses the record must call this so that the
     * client can be notified when the record is deleted.
     * @param pvRecordClient The client.
     * @return <b>true</b> if the client is added.
     */
    bool addPVRecordClient(PVRecordClientPtr const & pvRecordClient);
    /**
     * Remove a client.
     * @param pvRecordClient The client.
     * @return <b>true</b> if the client is removed.
     */
    bool removePVRecordClient(PVRecordClientPtr const & pvRecordClient);
    /**
     * remove all attached clients.
     */
    void detachClients();
    /**
     * Add a PVListener.
     * This must be called before calling pvRecordField.addListener.
     * @param pvListener The listener.
     * @return <b>true</b> if the listener was added.
     */
    bool addListener(PVListenerPtr const & pvListener);
    /**
     * Remove a listener.
     * @param pvListener The listener.
     * @return <b>true</b> if the listener was removed.
     */
    bool removeListener(PVListenerPtr const & pvListener);
    /**
     * Begins a group of puts.
     */
    void beginGroupPut();
    /**
     * Ends a group of puts.
     */
    void endGroupPut();
    /**
     * Virtual method of <b>Requester</b>
     * @return the name of the requester.
     */
    epics::pvData::String getRequesterName() {return getRecordName();}
    /**
     * Can be called by implementation code.
     * The message will be sent to every requester.
     * @param message The message.
     * @param messageType The severity of the message.
     */
    virtual void message(
        epics::pvData::String const & message,
        epics::pvData::MessageType messageType);
    /**
     * Called for a field. It will call the previous method
     * after adding field name.
     * @param pvRecordField The field for which the message is associated.
     * @param message The message.
     * @param messageType The severity of the message.
     */
    void message(
        PVRecordFieldPtr const & pvRecordField,
        epics::pvData::String const & message,
        epics::pvData::MessageType messageType);
    /**
     *  Calls the next method with indentLevel = 0.
     * @param buf String Builder.
     */
    void toString(epics::pvData::StringBuilder buf);
    /**
     * Dumps the data from the top level PVStructure. 
     * @param buf String Builder.
     * @param indentLevel The indentation level.
     */
    void toString(epics::pvData::StringBuilder buf,int indentLevel);
protected:
    /**
     * Constructor
     * @param recordName The name of the record
     * @param pvStructure The top level PVStructutre
     */
    PVRecord(
        epics::pvData::String const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    /**
     * Initializes the base class. Must be called by derived classes.
     */
    void initPVRecord();
    /**
     * Convience method for derived classes.
     * @return The shared pointer to the top level PVStructure.
     */
    epics::pvData::PVStructurePtr getPVStructure();
    PVRecordPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PVRecordFieldPtr findPVRecordField(
        PVRecordStructurePtr const & pvrs,
        epics::pvData::PVFieldPtr const & pvField);
    epics::pvData::String recordName;
    epics::pvData::PVStructurePtr pvStructure;
    epics::pvData::ConvertPtr convert;
    PVRecordStructurePtr pvRecordStructure;
    std::list<PVListenerPtr> pvListenerList;
    std::list<PVRecordClientPtr> pvRecordClientList;
    std::list<epics::pvData::RequesterPtr> requesterList;
    epics::pvData::Mutex mutex;
    epics::pvData::Lock thelock;
    std::size_t depthGroupPut;
    bool isDestroyed;
};

/**
 * Interface for a field of a record.
 * One exists for each field of the top level PVStructure.
 * @author mrk
 */
class PVRecordField :
     public virtual epics::pvData::PostHandler,
     public std::tr1::enable_shared_from_this<PVRecordField>
{
public:
    POINTER_DEFINITIONS(PVRecordField);
    /**
     * Constructor.
     * @param pvField The field from the top level structure.
     * @param The parent.
     */
    PVRecordField(
        epics::pvData::PVFieldPtr const & pvField,
        PVRecordStructurePtr const &parent,
        PVRecordPtr const & pvRecord);
    /**
     * Destructor.
     */
    virtual ~PVRecordField();
    /**
     *   Release any resources used
     */
    virtual void destroy();
    /**
     * Get the parent.
     * @return The parent.
     */
    PVRecordStructurePtr getParent();
    /**
     * Get the PVField.
     * @return The shared pointer.
     */
    epics::pvData::PVFieldPtr getPVField();
    /**
     * Get the full name of the field, i.e. field,field,..
     * @return The full name.
     */
    epics::pvData::String getFullFieldName();
    /**
     * Get the recordName plus the full name of the field, i.e. recordName.field,field,..
     * @return The name.
     */
    epics::pvData::String getFullName();
    /**
     * Returns the PVRecord to which this field belongs.
     * @return The shared pointer,
     */
    PVRecordPtr getPVRecord();
    /**
     * Add A PVListener to this field.
     * Whenever this field or any subfield if this field is modified the listener will be notified.
     * PVListener is described below.
     * Before a listener can call addListener it must first call PVRecord.registerListener.
     * @param pvListener The listener.
     * @return <b<true</b> if listener is added.
     */
    bool addListener(PVListenerPtr const & pvListener);
    /**
     * Remove a listener.
     * @param pvListener The listener.
     * @return <b<true</b> if listener is removed.
     */
    virtual void removeListener(PVListenerPtr const & pvListener);
    /**
     * This is called by the code that implements the data interface.
     * It is called whenever the put method is called.
     */
    virtual void postPut();
    /**
     * Called by implementation code.
     * It calls PVRecord::message after prepending the full fieldname.
     * @param message The message,
     * @param messageType The message severity.
     * @return
     */
    virtual void message(
        epics::pvData::String const & message,
        epics::pvData::MessageType messageType);
protected:
    PVRecordFieldPtr getPtrSelf()
    {
        return shared_from_this();
    }
    virtual void init();
private:
    void callListener();

    std::list<PVListenerPtr> pvListenerList;
    epics::pvData::PVFieldPtr pvField;
    PVRecordStructurePtr parent;
    PVRecordPtr pvRecord;
    epics::pvData::String fullName;
    epics::pvData::String fullFieldName;
    friend class PVRecordStructure;
    friend class PVRecord;
};

/**
 * Interface for a field that is a structure.
 * One exists for each structure field of the top level PVStructure.
 * @author mrk
 */
class PVRecordStructure : public PVRecordField {
public:
    POINTER_DEFINITIONS(PVRecordStructure);
    /**
     * Constructor.
     * @param pvStructure The data.
     * @param parent The parent
     * @param pvRecord The record that has this field.
     */
    PVRecordStructure(
        epics::pvData::PVStructurePtr const &pvStructure,
        PVRecordStructurePtr const &parent,
        PVRecordPtr const & pvRecord);
    /**
     * Destructor.
     */
    virtual ~PVRecordStructure();
    /**
     *   Release any resources used
     */
    virtual void destroy();
    /**
     * Get the sub fields.
     * @return the array of PVRecordFieldPtr.
     */
    PVRecordFieldPtrArrayPtr getPVRecordFields();
    /**
     * Get the data structure/
     * @return The shared pointer.
     */
    epics::pvData::PVStructurePtr getPVStructure();
    /**
     * Called by PVRecord::removeListener.
     * @param pvListener The listener.
     */
    virtual void removeListener(PVListenerPtr const & pvListener);
    /**
     * Called by implementation code of PVRecord.
     */
    virtual void postPut();
protected:
    /**
     * Called by implementation code of PVRecord.
     */
    virtual void init();
private:
    epics::pvData::PVStructurePtr pvStructure;
    PVRecordFieldPtrArrayPtr pvRecordFields;
    friend class PVRecord;
};

/**
 * An interface that must be implemented by any code that accesses the record.
 * @author mrk
 */
class PVRecordClient {
public:
    POINTER_DEFINITIONS(PVRecordClient);
    /**
     * Destructor.
     */
    virtual ~PVRecordClient() {}
    /**
     * Detach from the record because it is being removed.
     * @param pvRecord The record.
     */
    virtual void detach(PVRecordPtr const & pvRecord) = 0;
};

/**
 * An interface that is implemented by code that traps calls to PVRecord::message.
 * @author mrk
 */
class PVListener :
    virtual public PVRecordClient
{
public:
    POINTER_DEFINITIONS(PVListener);
    /**
     * Destructor.
     */
    virtual ~PVListener() {}
    /**
     * pvField has been modified.
     * This is called if the listener has called PVRecordField::addListener for pvRecordField.
     * @param pvRecordField The modified field.
     */
    virtual void dataPut(PVRecordFieldPtr const & pvRecordField) = 0;
    /**
     * A subfield has been modified.
     * @param requested The structure that was requested.
     * @param pvRecordField The field that was modified.
     */
    virtual void dataPut(
        PVRecordStructurePtr const & requested,
        PVRecordFieldPtr const & pvRecordField) = 0;
    /**
     * Begin a set of puts.
     * @param pvRecord The record.
     */
    virtual void beginGroupPut(PVRecordPtr const & pvRecord) = 0;
    /**
     * End a set of puts.
     * @param pvRecord The record.
     */
    virtual void endGroupPut(PVRecordPtr const & pvRecord) = 0;
};

/**
 * The interface to a database of PVRecords.
 * @author mrk
 */
class PVDatabase : virtual public epics::pvData::Requester {
public:
    POINTER_DEFINITIONS(PVDatabase);
    /**
     * Get the master database.
     * @return The shared pointer.
     */
    static PVDatabasePtr getMaster();
    /**
     * Destructor
     */
    virtual ~PVDatabase();
    /**
     * Find a record.
     * An empty pointer is returned if the record is not in the database.
     * @param recordName The record to find.
     * @return The shared pointer.
     */
    PVRecordPtr findRecord(epics::pvData::String const& recordName);
    /**
     * Add a record.
     * @param The record to add.
     * @return <b>true</b> if record was added.
     */
    bool addRecord(PVRecordPtr const & record);
    /**
     * Remove a record.
     * @param The record to remove.
     * @return <b>true</b> if record was removed.
     */
    bool removeRecord(PVRecordPtr const & record);
    /**
     * Virtual method of Requester.
     * @return The name.
     */
    virtual epics::pvData::String getRequesterName();
    /**
     * Virtual method of Requester.
     * @param message The message.
     * @param messageType The message severity.
     * @return
     */
    virtual void message(
        epics::pvData::String const & message,
        epics::pvData::MessageType messageType);
private:
    PVDatabase();
    PVRecordMap  recordMap;
    
};

}}

#endif  /* PVDATABASE_H */
