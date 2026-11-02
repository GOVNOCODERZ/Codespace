unit Unit2;

interface

uses
  System.SysUtils, System.Classes, Data.DB, Data.Win.ADODB;

type
  TDataModule2 = class(TDataModule)
    ADOConnection1: TADOConnection;
    T_goods_catalog: TADOTable;
    T_type_goods: TADOTable;
    T_suppliers_catalog: TADOTable;
    T_operations: TADOTable;
    T_op_goods_list: TADOTable;
    T_storage: TADOTable;
    Q_v_goods_catalog: TADOQuery;
    Q_v_goods_operations: TADOQuery;
    Q_v_storage: TADOQuery;
    DS_T_goods_catalog: TDataSource;
    DS_T_type_goods: TDataSource;
    DS_T_suppliers_catalog: TDataSource;
    DS_T_operations: TDataSource;
    DS_T_op_goods_list: TDataSource;
    DS_T_storage: TDataSource;
    DS_Q_v_storage: TDataSource;
    DS_Q_v_goods_operations: TDataSource;
    DS_Q_v_goods_catalog: TDataSource;
    Q_v_goods_catalogIDgc: TAutoIncField;
    Q_v_goods_cataloggcName: TWideStringField;
    Q_v_goods_catalogtgName: TWideStringField;
    Q_v_goods_cataloggcDescription: TWideStringField;
    Q_v_goods_cataloggcPhoto: TBlobField;
    Q_v_goods_cataloggcCost: TBCDField;
    procedure Q_v_goods_catalogAfterScroll(DataSet: TDataSet);
    procedure T_goods_catalogAfterDelete(DataSet: TDataSet);
    procedure T_goods_catalogAfterPost(DataSet: TDataSet);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  DataModule2: TDataModule2;

implementation

{%CLASSGROUP 'Vcl.Controls.TControl'}

uses Unit3;

{$R *.dfm}

procedure TDataModule2.Q_v_goods_catalogAfterScroll(DataSet:
TDataSet);
begin
// проверка количества отображаемых записей
if Q_v_goods_catalog.RecordCount > 0 then
// выполнение синхронизации методом Locate
T_goods_catalog.Locate('IDgc', Q_v_goods_catalogIDgc.Value, [])
end;

procedure TDataModule2.T_goods_catalogAfterDelete(DataSet: TDataSet);
begin
// обновление данных в НД Q_v_goods_catalog
Q_v_goods_catalog.Requery();
// обновление данных в НД Q_v_storage
Q_v_storage.Requery();
end;
procedure TDataModule2.T_goods_catalogAfterPost(DataSet: TDataSet);
var pos:TBookmark;
begin
// создание для текущей записи объекта-закладки
pos := Q_v_goods_catalog.GetBookmark;
Q_v_goods_catalog.Close();
Q_v_goods_catalog.Open();
// перемещение курсора БД на запись, определяемую закладкой
Q_v_goods_catalog.GotoBookmark(pos);
Q_v_storage.Requery();
end;

end.
