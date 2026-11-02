unit Unit4;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.ComCtrls, Data.DB, Vcl.Grids,
  Vcl.DBGrids, Vcl.DBCtrls, Vcl.StdCtrls, Vcl.ExtCtrls, Vcl.Mask;

type
  TForm4 = class(TForm)
    PageControl1: TPageControl;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    DBGrid1: TDBGrid;
    DBGrid2: TDBGrid;
    Label1: TLabel;
    DBText1: TDBText;
    InsertEditDeletePanel: TPanel;
    PanelToInputValues: TPanel;
    InsertButton: TButton;
    EditButton: TButton;
    DeleteButton: TButton;
    DBLookupComboBox1: TDBLookupComboBox;
    DBEdit1: TDBEdit;
    DBEdit2: TDBEdit;
    DBEdit3: TDBEdit;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    PostButton: TButton;
    CancelButton: TButton;
    procedure InsertButtonClick(Sender: TObject);
    procedure EditButtonClick(Sender: TObject);
    procedure DeleteButtonClick(Sender: TObject);
    procedure PostButtonClick(Sender: TObject);
    procedure CancelButtonClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form4: TForm4;

implementation

{$R *.dfm}

uses Unit2, Unit1, Unit3;

procedure TForm4.CancelButtonClick(Sender: TObject);
begin
if DataModule2.T_goods_catalog.State in [dsInsert, dsEdit] then
begin
// Выполним операцию отмены
DataModule2.T_goods_catalog.Cancel;
InsertEditDeletePanel.Visible := True;
PanelToInputValues.Visible := False;
DBGrid1.Enabled := True;
end;
end;

procedure TForm4.DeleteButtonClick(Sender: TObject);
begin
if DataModule2.T_goods_catalog.State = dsBrowse then
if MessageDlg('Подтвердите удаление записи', mtConfirmation,
[mbYes, mbNo], 0) = mrYes then
DataModule2.T_goods_catalog.Delete;
end;

procedure TForm4.EditButtonClick(Sender: TObject);
begin
if DataModule2.T_goods_catalog.State = dsBrowse then
begin
DBGrid1.Enabled := False;
InsertEditDeletePanel.Visible := False;
PanelToInputValues.Visible := True;
// Переведем НД в режим редактирования записи, используя
// метод Edit
DataModule2.T_goods_catalog.Edit;
end;
end;

procedure TForm4.InsertButtonClick(Sender: TObject);
begin
if DataModule2.T_goods_catalog.State = dsBrowse then
begin
// Отключим возможность изменения курсора НД в компоненте DBGrid1
DBGrid1.Enabled := False;
// Сделаем невидимой панель с кнопками Вставить, Изменить,
// Удалить
InsertEditDeletePanel.Visible := False;
// Сделаем видимой панель для ввода значения в запись
PanelToInputValues.Visible := True;
// Переведем НД в режим добавления записи, используя
// метод Append
DataModule2.T_goods_catalog.Append;
end;
end;

procedure TForm4.PostButtonClick(Sender: TObject);
begin
if DataModule2.T_goods_catalog.State in [dsInsert, dsEdit] then
begin
// Выполним операцию сохранения
DataModule2.T_goods_catalog.Post;
// Сделаем видимой панель с кнопками Вставить, Изменить,
// Удалить
InsertEditDeletePanel.Visible := True;
// Сделаем невидимой панель для ввода значения в запись
PanelToInputValues.Visible := False;
// Включим возможность изменения курсора НД
// в компоненте DBGrid1
DBGrid1.Enabled := True;
end;
end;

end.
