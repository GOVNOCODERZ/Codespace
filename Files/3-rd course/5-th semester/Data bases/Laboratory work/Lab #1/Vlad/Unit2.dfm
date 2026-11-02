object DataModule2: TDataModule2
  Height = 774
  Width = 1082
  object ADOConnection1: TADOConnection
    Connected = True
    ConnectionString = 
      'Provider=MSDASQL.1;Persist Security Info=False;Extended Properti' +
      'es="CHARSET=utf8mb4;COLUMN_SIZE_S32=1;DATABASE=goodsmanager;DB=g' +
      'oodsmanager;DRIVER=MySQL ODBC 8.0 Unicode Driver;NO_SCHEMA=1;POR' +
      'T=3306;PREFETCH=;SERVER=localhost;UID=root;USER=root"'
    Left = 152
    Top = 128
  end
  object T_goods_catalog: TADOTable
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    AfterOpen = T_goods_catalogAfterOpen
    AfterPost = T_goods_catalogAfterPost
    AfterDelete = T_goods_catalogAfterDelete
    TableName = 'goods_catalog'
    Left = 320
    Top = 112
  end
  object T_type_goods: TADOTable
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    TableName = 'type_goods'
    Left = 320
    Top = 192
  end
  object T_suppliers_catalog: TADOTable
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    TableName = 'suppliers_catalog'
    Left = 320
    Top = 272
  end
  object T_operations: TADOTable
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    TableName = 'operations'
    Left = 320
    Top = 352
  end
  object T_op_goods_list: TADOTable
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    TableName = 'op_goods_list'
    Left = 320
    Top = 432
  end
  object T_storage: TADOTable
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    TableName = 'storage'
    Left = 320
    Top = 512
  end
  object Q_v_goods_catalog: TADOQuery
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    AfterPost = Q_v_goods_catalogAfterPost
    AfterDelete = Q_v_goods_catalogAfterDelete
    AfterScroll = Q_v_goods_catalogAfterScroll
    Parameters = <>
    SQL.Strings = (
      'SELECT *'
      'FROM view_goods_catalog')
    Left = 832
    Top = 144
    object Q_v_goods_catalogIDgc: TAutoIncField
      FieldName = 'IDgc'
      ReadOnly = True
    end
    object Q_v_goods_cataloggcName: TWideStringField
      FieldName = 'gcName'
      Size = 100
    end
    object Q_v_goods_catalogtgName: TWideStringField
      FieldName = 'tgName'
      Size = 50
    end
    object Q_v_goods_cataloggcDescription: TWideStringField
      FieldName = 'gcDescription'
      Size = 255
    end
    object Q_v_goods_cataloggcPhoto: TBlobField
      FieldName = 'gcPhoto'
    end
    object Q_v_goods_cataloggcCost: TBCDField
      FieldName = 'gcCost'
      Precision = 10
      Size = 2
    end
  end
  object Q_v_goods_operations: TADOQuery
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    Parameters = <>
    SQL.Strings = (
      'SELECT *'
      'FROM view_goods_operations')
    Left = 832
    Top = 232
  end
  object Q_v_storage: TADOQuery
    Active = True
    Connection = ADOConnection1
    CursorType = ctStatic
    Parameters = <>
    SQL.Strings = (
      'SELECT *'
      'FROM view_storage')
    Left = 832
    Top = 328
  end
  object DS_T_goods_catalog: TDataSource
    DataSet = T_goods_catalog
    Left = 484
    Top = 132
  end
  object DS_T_type_goods: TDataSource
    DataSet = T_type_goods
    Left = 464
    Top = 216
  end
  object DS_T_suppliers_catalog: TDataSource
    DataSet = T_suppliers_catalog
    Left = 472
    Top = 312
  end
  object DS_T_operations: TDataSource
    DataSet = T_operations
    Left = 464
    Top = 392
  end
  object DS_T_op_goods_list: TDataSource
    DataSet = T_op_goods_list
    Left = 472
    Top = 464
  end
  object DS_T_storage: TDataSource
    DataSet = T_storage
    Left = 472
    Top = 552
  end
  object DS_Q_v_storage: TDataSource
    DataSet = Q_v_storage
    Left = 952
    Top = 320
  end
  object DS_Q_v_goods_operations: TDataSource
    DataSet = Q_v_goods_operations
    Left = 968
    Top = 232
  end
  object DS_Q_v_goods_catalog: TDataSource
    DataSet = Q_v_goods_catalog
    Left = 960
    Top = 144
  end
end
