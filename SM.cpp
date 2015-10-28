#include "SM.h"
#include "zbase.h"

SM_Manager::SM_Manager(IX_Manager &ixm, RM_Manager &rmm): ixm(ixm), rmm(rmm) {

}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::OpenDb(const string &dbName) {
	RC rc;

	if((rc = rmm.OpenFile("relcat", relfh)) != 0)
		return rc;
	if((rc = rmm.OpenFile("attrcat", attrfh)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::CloseDb() {
	RC rc;

	if((rc = rmm.CloseFile(relfh)) != 0)
		return rc;
	if((rc = rmm.CloseFile(attrfh)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::CreateTable(const string &relationName, const vector<AttrInfo> &attrs) {
	int size = 0;
	RID rid;
	RC rc;
	RelationCatRecord relRecord;
	AttrCatRecord attrRecord;

	for(int i = 0; i < attrs.size(); i++) {
		attrRecord.relationName = relationName;
		attrRecord.attrName = attrs[i].attrName;
		attrRecord.offset = size;
		attrRecord.attrType = attrs[i].attrType;
		attrRecord.attrLength = attrs[i].attrLength;
		attrRecord.indexNo = -1;
		if((rc = attrfh.InsertRecord((char*) &attrRecord, rid)) != 0)
			return rc;
		size += attrs[i].attrLength;
	}

	if((rc = rmm.CreateFile(relationName, size)) != 0)
		return rc;

	relRecord.relationName = relationName;
	relRecord.tupleLength = size;
	relRecord.attrCount = attrs.size();
	relRecord.indexCount = 0;
	if((rc = relfh.InsertRecord((char *) &relRecord, rid)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::DropTable(const string &relationName) {
	RM_FileScan rmfs;
	RC rc;
	RID rid;
	bool isFound = false;
	RM_Record rec;
	RelationCatRecord *relRecord;
	AttrCatRecord *attrRecord;

	if((rc = rmfs.OpenScan(relfh, CHARN, MAXRELATIONNAME, offsetof(RelationCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0 )
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData((char*&) relRecord);
		if(relRecord->relationName == relationName) {
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	if((rc = rmm.DestoryFile(relationName)) != 0)
		return rc;

	rec.GetRid(rid);
	if((rc = relfh.DeleteRecord(rid)) != 0)
		return rc;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXRELATIONNAME, offsetof(AttrCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0)
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData((char*&) attrRecord);
		if(attrRecord->relationName == relationName) {
			if(attrRecord->indexNo != -1)
				DropIndex(relationName, attrRecord->attrName);
			rec.GetRid(rid);
			if((rc = attrfh.DeleteRecord(rid)) != 0)
				return rc;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	return 0;
}

RC SM_Manager::CreateIndex(const string &relationName, const string &attrName) {
	RC rc;
	RID rid;
	AttrCatRecord attrRecord;
	RelationCatRecord relRecord;
	RM_Record rec;
	IX_IndexHandle ixh;
	RM_FileHandle rmfh;
	RM_FileScan rmfs;
	char *pdata;

	if((rc = GetAttrInfo(relationName, attrName, attrRecord, rid)) != 0)
		return rc;

	if(attrRecord.indexNo != -1)
		return SM_INDEXEXISTS;

	attrRecord.indexNo = attrRecord.offset;
	if((rc = attrfh.GetRecord(rid, rec)) != 0)
		return rc;
	if((rc = attrfh.UpdateRecord(rec)) != 0)
		return rc;

	if((rc = GetRelationInfo(relationName, relRecord, rid)) != 0)
		return rc;
	relRecord.indexCount++;
	if((rc = relfh.GetRecord(rid, rec)) != 0)
		return rc;
	if((rc = relfh.UpdateRecord(rec)) != 0)
		return rc;

	if((rc = ixm.CreateIndex(relationName, attrRecord.indexNo, attrRecord.attrType, attrRecord.attrLength)) != 0)
		return rc;
	if((rc = ixm.OpenIndex(relationName, attrRecord.indexNo, ixh)) != 0)
		return rc;
	if((rc = rmm.OpenFile(relationName, rmfh)) != 0)
		return rc;
	if((rc = rmfs.OpenScan(rmfh, attrRecord.attrType, attrRecord.attrLength, attrRecord.offset, NO, NULL)) != 0)
		return rc;

	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(pdata);
		rec.GetRid(rid);
		ixh.InsertEntry(pdata + attrRecord.offset, rid);
	}

	if((rc = rmfs.CloseScan()) != 0)
		return rc;
	if((rc = rmm.CloseFile(rmfh)) != 0)
		return rc;
	if((rc = ixm.CloseIndxe(ixh)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::DropIndex(const string &relationName, const string &attrName) {
	RM_FileScan rmfs;
	bool isFound = false;
	RM_Record rec;
	RelationCatRecord relRecord;
	AttrCatRecord *attrRecord;
	RID rid;
	RC rc;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXRELATIONNAME, offsetof(AttrCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0)
		return rc;

	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData((char*&) attrRecord);
		if(attrRecord->attrName == attrName) {
			attrRecord->indexNo = -1;
			isFound = true;
			break;
		}
	}

	if((rc = rmfs.CloseScan()) != 0)
		return rc;
	if(isFound == false)
		return SM_NOTFOUND;

	rec.GetRid(rid);
	if((rc = ixm.DestoryIndex(relationName, attrRecord->offset)))
		return rc;

	if((rc = attrfh.UpdateRecord(rec)) != 0)
		return rc;

	if((rc = GetRelationInfo(relationName, relRecord, rid)) != 0)
		return rc;
	relRecord.indexCount--;
	if((rc = relfh.GetRecord(rid, rec)) != 0)
		return rc;
	if((rc = relfh.UpdateRecord(rec)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::GetAttrInfo(const string &relationName, int attrCount, vector<AttrCatRecord> &attrs) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	RelationCatRecord *relRecord;
	AttrCatRecord *attrRecord;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXRELATIONNAME, offsetof(AttrCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0)
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData((char*&) attrRecord);
		if(attrRecord->relationName == relationName) {
			attrs.push_back(*attrRecord);
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	return 0;
}

RC SM_Manager::GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData, RID &rid) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	AttrCatRecord *attrRecord;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXRELATIONNAME, offsetof(AttrCatRecord, attrName), EQ, (void*) attrName.c_str())) != 0)
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData((char*&) attrRecord);
		if(attrRecord->attrName == attrName) {
			rec.GetRid(rid);
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	attrData = *attrRecord;

	return 0;
}

RC SM_Manager::GetRelationInfo(const string &relationName, RelationCatRecord &relationData, RID &rid) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	RelationCatRecord *relRecord;

	if((rc = rmfs.OpenScan(relfh, CHARN, MAXRELATIONNAME, offsetof(RelationCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0 )
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData((char*&) relRecord);
		if(relRecord->relationName == relationName) {
			rec.GetRid(rid);
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	relationData = *relRecord;

	return 0;
}